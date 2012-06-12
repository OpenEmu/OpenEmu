/*
 * ide.c - Emulate IDE interface
 *
 * Copyright (C) 2010 Ivo van Poorten
 *
 * Based on QEMU IDE disk emulator (hw/ide/{core.c,microdrive.c,mmio.c})
 * Copyright (C) 2003 Fabrice Bellard
 * Copyright (C) 2006 Openhand Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
*/

/* Emulate 8-bit and 16-bit IDE interface at $D500-$D50F
 *
 * Eight 16-bit IDE registers:
 *
 * LSB (bit 0..7)  at 0xd500-0xd507
 * MSB (bit 8..15) at 0xd508-0xd50f
 *
 * Full 16-bits are read and written when accessing the LSB (latch)
 *
 * If the MSB is ignored, the interface is compatible with MyIDE
 *
 * 16-bit operations:
 *
 * Read:    read LSB (device returns 1 16-bit word), read MSB
 * Write:   write MSB, write LSB (16-bit word is written to device)
 */

#define _XOPEN_SOURCE 500

#include "config.h"
/* allow non-ansi fseek/ftell functions */
#ifdef __STRICT_ANSI__
#  undef __STRICT_ANSI__
#  include <stdio.h>
#  define __STRICT_ANSI__ 1
#else
#  include <stdio.h>
#endif
#include "ide.h"
#include "atari.h"
#include "log.h"
#include "util.h"
#include "ide_internal.h"

#include <string.h>
#ifdef HAVE_INTTYPES_H
#  include <inttypes.h>
#endif
#include <stdlib.h>
#include <errno.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#define SECTOR_SIZE 512
#define STD_HEADS   16          
#define STD_SECTORS 63

#if defined (HAVE_WINDOWS_H)
#  define fseeko fseeko64
#  define ftello ftello64
#elif defined (__BEOS__)
#  define fseeko _fseek
#  define ftello _ftell
#  define PRId64 "lld"
#elif defined (__DJGPP__)
#  define fseeko fseek
#  define ftello ftell
#  define PRId64 "lld"
#endif

int IDE_enabled = 0, IDE_debug = 0;

struct ide_device device;

static int count = 0;     /* for debug stuff */

static inline void padstr(uint8_t *str, const char *src, int len) {
    int i;
    for(i = 0; i < len; i++)
        str[i^1] = *src ? *src++ : ' ';
}

#define LE16(x,y,z) (x)[(y)<<1] = (z)&0xff; (x)[((y)<<1)+1] = ((uint16_t)z)>>8;

static void ide_identify(struct ide_device *s) {
    unsigned int oldsize;
    uint8_t *p = s->io_buffer;
    memset(p, 0, 512);

    LE16(p, 0, GCBI_FIXED_DRIVE);
    LE16(p, 1, s->cylinders);
    LE16(p, 3, s->heads);
/*
    LE16(p, 4, 512 * s->sectors);   // bytes per track, obsolete ATA2
    LE16(p, 5, 512);                // bytes per sector, obsolete ATA2
*/
    LE16(p, 6, s->sectors);         /* sectors per track */

    padstr(p+10*2, s->drive_serial_str, 20);

/*
    LE16(p, 20, 3);                 // buffer type, obsolete ATA2
    LE16(p, 21, 16);                // cache size in sectors, obsolete ATA2
*/
    LE16(p, 22, 4);                 /* number of ECC bytes */

    padstr(p+23*2, PACKAGE_VERSION, 8);
    padstr(p+27*2, "ATARI800 HARDDISK", 40);

    if (MAX_MULT_SECTORS > 1)
        LE16(p, 47, 0x8000 | MAX_MULT_SECTORS);

    LE16(p, 48, 0);                 /* cannot perform double word I/O */
    LE16(p, 49, CAP_LBA_SUPPORTED);
    LE16(p, 51, 0x0200);            /* PIO transfer cycle */
/*
    LE16(p, 52, 0x0200);            // DMA transfer cycle, obsolete ATA3
*/
    LE16(p, 53, 1/*+2+4*/);         /* words 54-58[,64-70,88] are valid */
    LE16(p, 54, s->cylinders);
    LE16(p, 55, s->heads);
    LE16(p, 56, s->sectors);
    oldsize = s->cylinders * s->heads * s->sectors;
    LE16(p, 57, oldsize);
    LE16(p, 58, oldsize >> 16);
    if (s->mult_sectors)
        LE16(p, 59, 0x100 | s->mult_sectors);

    LE16(p, 60, s->nb_sectors);     /* total number of LBA sectors */
    LE16(p, 61, s->nb_sectors >> 16);

    if (s->is_cf) {
        LE16(p, 0, 0x848a);         /* CF Storage Card signature */
        padstr(p+27*2, "ATARI800 MICRODRIVE", 40);
        LE16(p, 49, CAP_LBA_SUPPORTED);
        LE16(p, 51, 2);
        LE16(p, 52, 1);
    }

    if (s->is_cdrom) {
        LE16(p, 0, GCBI_HAS_PACKET_FEAT_SET |
                   GCBI_CDROM_DEVICE        |
                   GCBI_HAS_REMOVABLE_MEDIA |
                   GCBI_50US_TILL_DRQ       |
                   GCBI_12BYTE_PACKETS);
        padstr(p+27*2, "ATARI800 DVD-ROM", 40);
        LE16(p, 49, CAP_LBA_SUPPORTED);
    }
}

static void ide_dummy_transfer_stop(struct ide_device *s) {
    s->data_ptr = s->data_end = s->io_buffer;
    s->io_buffer[0] = s->io_buffer[1] = 
    s->io_buffer[2] = s->io_buffer[3] = 0xff;
    count = 0;
}

static void ide_reset(struct ide_device *s) {
    if (IDE_debug) fprintf(stderr, "ide: reset\n");

    if (s->is_cf) s->mult_sectors = 0;
    else          s->mult_sectors = MAX_MULT_SECTORS;

    /* ide regs */
    s->feature = s->error = s->nsector = s->sector = s->lcyl = s->hcyl = 0;

    /* lba48 */
    s->hob_feature = s->hob_sector = s->hob_nsector =
    s->hob_lcyl = s->hob_hcyl = s->lba48 = 0;

    s->select = 0xa0;
    s->status = READY_STAT | SEEK_STAT;

    /* ATAPI stuff skipped */

    s->select &= 0xf0;  /* clear head */
    s->nsector = s->sector = 1;
    if (s->is_cdrom) {
        s->lcyl = 0x14;
        s->hcyl = 0xeb;
    } else {
        s->lcyl = s->hcyl = 0;
    }

    s->end_transfer_func = ide_dummy_transfer_stop;
    ide_dummy_transfer_stop(s);
    s->media_changed = 0;
}

static int ide_init_drive(struct ide_device *s, char *filename) {
    if (!(s->file = fopen(filename, "rb+"))) {
        Log_print("%s: %s", filename, strerror(errno));
        return FALSE;
    }

    s->blocksize = SECTOR_SIZE;

    fseeko(s->file, 0, SEEK_END);
    s->filesize = ftello(s->file);

    if (IDE_debug)
        fprintf(stderr, "ide: filesize: %"PRId64"\n", (int64_t)s->filesize);

    if (!s->io_buffer) {
        s->io_buffer_size = SECTOR_SIZE * MAX_MULT_SECTORS;
        s->io_buffer      = Util_malloc(s->io_buffer_size);
    }

    s->nb_sectors = s->filesize / SECTOR_SIZE;

    /* use standard physical disk geometry */
    s->cylinders = s->nb_sectors / (STD_HEADS * STD_SECTORS);

    if (s->cylinders > 16383)
        s->cylinders = 16383;
    else if (s->cylinders < 2) {
        Log_print("%s: image file too small\n", filename);
        return FALSE;
    }

    s->heads   = STD_HEADS;
    s->sectors = STD_SECTORS;

    if (IDE_debug)
        fprintf(stderr, "ide: cyls/heads/secs - %d/%d/%d\n", 
                s->cylinders, s->heads, s->sectors);

    s->drive_serial = 1;
    snprintf(s->drive_serial_str, sizeof(s->drive_serial_str),
            "QM%05d", s->drive_serial);

    ide_reset(s);

    return TRUE;
}

static uint32_t ide_ioport_read(struct ide_device *s, uint16_t addr) {
    int ret = 0xff, hob = 0;

    addr &= 7;
    /* hob   = s->select & (1<<7); */
    /* FIXME: HOB stuff is broken/disabled in QEMU */

    switch(addr) {
    case 0: /* bottom ide layer does nothing here */        break;
    case 1: ret = hob ? s->hob_feature : s->error;          break;
    case 2: ret = hob ? s->hob_nsector : s->nsector & 0xff; break;
    case 3: ret = hob ? s->hob_sector  : s->sector;         break;
    case 4: ret = hob ? s->hob_lcyl    : s->lcyl;           break;
    case 5: ret = hob ? s->hob_hcyl    : s->hcyl;           break;
    case 6: ret = s->select;                                break;
    default:
    case 7: ret = s->status;                                break;
    }

    if (IDE_debug)
        fprintf(stderr, "ide: get: addr: %04x, ret: %02x\n", addr, ret);

    return ret;
}

static inline void not_implemented(uint8_t val) {
    if (IDE_debug) fprintf(stderr, "\tIDE: %02x not implemented\n", val);
}

static inline void ide_abort_command(struct ide_device *s) {
    s->status = READY_STAT | ERR_STAT;
    s->error  = ABRT_ERR;
}

static void ide_cmd_lba48_transform(struct ide_device *s, int lba48) {
    s->lba48 = lba48;
    if (!s->lba48) {
        if (!s->nsector)
            s->nsector = 256;
    } else {
        if (!s->nsector && !s->hob_nsector)
            s->nsector = 65536;
        else      
            s->nsector = (s->hob_nsector << 8) | s->nsector;
    }
}
static int64_t ide_get_sector(struct ide_device *s) {
    int64_t sector_num;
    if (s->select & 0x40) {         /* lba */
        if (IDE_debug)
            fprintf(stderr, "get_sector: lba\n");
        if (!s->lba48) {
            sector_num = ((s->select & 0x0f) << 24) |
                         ( s->hcyl           << 16) |
                         ( s->lcyl           <<  8) | s->sector;
        } else {
            sector_num = ((int64_t) s->hob_hcyl   << 40) |
                         ((int64_t) s->hob_lcyl   << 32) |
                         ((int64_t) s->hob_sector << 24) |
                         ((int64_t) s->hcyl       << 16) |
                         ((int64_t) s->lcyl       <<  8) | s->sector;
        }
    } else {
        sector_num = ((s->hcyl << 8) | s->lcyl) * s->heads * s->sectors +
                      (s->select & 0x0f) * s->sectors + 
                      (s->sector - 1);

        if (IDE_debug)
            fprintf(stderr, "get_sector: large: hcyl %02x  lcyl %02x  heads %02x  sectors %02x  select&f %1x  sector-1 %d  sector_num %"PRId64"\n", s->hcyl, s->lcyl, s->heads, s->sectors, s->select&0x0f, s->sector-1, sector_num);

    }
    return sector_num;
}

static void ide_transfer_start(struct ide_device *s, uint8_t *buf, int size,
                                       EndTransferFunc *end_transfer_func) {
    if (IDE_debug) fprintf(stderr, "transfer start\n");

    s->end_transfer_func = end_transfer_func;
    s->data_ptr          = buf;
    s->data_end          = buf + size;
    s->cycle             = 0;

    if (!(s->status & ERR_STAT)) s->status |= DRQ_STAT;
}

static void ide_transfer_stop(struct ide_device *s) {
    if (IDE_debug) fprintf(stderr, "transfer stop\n");

    s->end_transfer_func = ide_transfer_stop;
    s->data_ptr          = s->io_buffer;
    s->data_end          = s->io_buffer;
    s->status           &= ~DRQ_STAT;
    count                = 0;
}

static void ide_set_sector(struct ide_device *s, int64_t sector_num) {
    unsigned int cyl, r;

    if (s->select & 0x40) {
        if (!s->lba48) {
            s->select = (s->select & 0xf0) | (sector_num >> 24);
            s->hcyl   =                       sector_num >> 16 ;
            s->lcyl   =                       sector_num >>  8 ;
            s->sector =                       sector_num       ;
        } else {
            s->sector     = sector_num      ;
            s->lcyl       = sector_num >>  8;
            s->hcyl       = sector_num >> 16;
            s->hob_sector = sector_num >> 24;
            s->hob_lcyl   = sector_num >> 32;
            s->hob_hcyl   = sector_num >> 40;
        }
    } else {
        cyl = sector_num / (s->heads * s->sectors);
        r   = sector_num % (s->heads * s->sectors);

        s->hcyl   = cyl >> 8;
        s->lcyl   = cyl     ;
        s->select = (s->select & 0xf0) | ((r / s->sectors) & 0x0f);
        s->sector =                       (r % s->sectors) + 1;
    }
}

static void ide_sector_read(struct ide_device *s) {
    int64_t sector_num;
    int n;

    s->status  = READY_STAT | SEEK_STAT;
    s->error   = 0;
    sector_num = ide_get_sector(s);
    n          = s->nsector;

    if (!n) {
        ide_transfer_stop(s);
    } else {
        if (IDE_debug)
            fprintf(stderr, "IDE: read sector=%" PRId64 "\n", sector_num);

        if (n > s->req_nb_sectors)
            n = s->req_nb_sectors;

        if (fseeko(s->file, sector_num * SECTOR_SIZE, SEEK_SET) < 0)
            goto fail;
        if (fread(s->io_buffer, n * SECTOR_SIZE, 1, s->file) != 1)
            goto fail;

        if (IDE_debug) fprintf(stderr, "sector read OK\n");

        ide_transfer_start(s, s->io_buffer, 512*n, ide_sector_read);
        s->nsector -= n;
        ide_set_sector(s, sector_num + n + (s->nsector ? 0 : -1));
    }
    return;

fail:
    ide_abort_command(s);
    if (IDE_debug) fprintf(stderr, "sector read FAILED\n");
}

static void ide_sector_write(struct ide_device *s) {
    int64_t sector_num;
    int n, n1;

    s->status  = READY_STAT | SEEK_STAT;
    sector_num = ide_get_sector(s);

    if (IDE_debug)
        fprintf(stderr, "IDE: write sector=%" PRId64 "\n", sector_num);

    n = s->nsector;
    if (n > s->req_nb_sectors)
        n = s->req_nb_sectors;

    if (fseeko(s->file, sector_num * SECTOR_SIZE, SEEK_SET) < 0) {
        fprintf(stderr, "FSEEKO FAILED\n");
        goto fail;
    }
    if (fwrite(s->io_buffer, n * SECTOR_SIZE, 1, s->file) != 1) {
        fprintf(stderr, "FWRITE FAILED\n");
        goto fail;
    }
    fflush(s->file);

    s->nsector -= n;
    if (s->nsector == 0) {
        ide_transfer_stop(s);
    } else {
        n1 = s->nsector;
        if (n1 > s->req_nb_sectors)
            n1 = s->req_nb_sectors;
        ide_transfer_start(s, s->io_buffer, 512 * n1, ide_sector_write);
    }
    ide_set_sector(s, sector_num + n + (s->nsector ? 0 : -1));
    return;

fail:
    ide_abort_command(s);
}

static void ide_command(struct ide_device *s, uint8_t val) {
    int lba48 = 0, n;

    switch(val) {
    case WIN_IDENTIFY:
        ide_identify(s);
        s->status = READY_STAT | SEEK_STAT;
        ide_transfer_start(s, s->io_buffer, 512, ide_transfer_stop);
        break;

    case WIN_SPECIFY:
    case WIN_RECAL:
        s->error = 0;
        s->status = READY_STAT | SEEK_STAT;
        break;

    case WIN_SETMULT:
        if (s->is_cf && !s->nsector) {
            s->mult_sectors = 0;
            s->status = READY_STAT | SEEK_STAT;
        } else if ((s->nsector & 0xff) != 0
               && ((s->nsector & 0xff) > MAX_MULT_SECTORS
               ||  (s->nsector & (s->nsector - 1)) != 0)) {
            ide_abort_command(s);
        } else {
            s->mult_sectors = s->nsector & 0xff;
            s->status       = READY_STAT | SEEK_STAT;
        } 
        break;

    case WIN_VERIFY_EXT:
        lba48 = 1;
    case WIN_VERIFY:
    case WIN_VERIFY_ONCE:
        /* do sector number check ? */
        ide_cmd_lba48_transform(s, lba48);
        s->status = READY_STAT | SEEK_STAT;
        break;

    case WIN_READ_EXT:
        lba48 = 1;
    case WIN_READ:
    case WIN_READ_ONCE:
        ide_cmd_lba48_transform(s, lba48);
        s->req_nb_sectors = 1;
        ide_sector_read(s);
        break;

    case WIN_WRITE_EXT:
        lba48 = 1;
    case WIN_WRITE:
    case WIN_WRITE_ONCE:
    case CFA_WRITE_SECT_WO_ERASE:
    case WIN_WRITE_VERIFY:
        ide_cmd_lba48_transform(s, lba48);
        s->error          = 0;
        s->status         = SEEK_STAT | READY_STAT;
        s->req_nb_sectors = 1;
        ide_transfer_start(s, s->io_buffer, 512, ide_sector_write);
        s->media_changed = 1;
        break;

    case WIN_MULTREAD_EXT:
        lba48 = 1;
    case WIN_MULTREAD:
        if (!s->mult_sectors) goto abort_cmd;

        ide_cmd_lba48_transform(s, lba48);
        s->req_nb_sectors = s->mult_sectors;
        ide_sector_read(s);
        break;

    case WIN_MULTWRITE_EXT:
        lba48 = 1;
    case WIN_MULTWRITE:
    case CFA_WRITE_MULTI_WO_ERASE:
        if (!s->mult_sectors) goto abort_cmd;

        ide_cmd_lba48_transform(s, lba48);
        s->error          = 0;
        s->status         = SEEK_STAT | READY_STAT;
        s->req_nb_sectors = s->mult_sectors;
        n                 = s->nsector;
        if (n > s->req_nb_sectors)
            n = s->req_nb_sectors;
        ide_transfer_start(s, s->io_buffer, 512 * n, ide_sector_write);
        s->media_changed = 1;
        break;

    case WIN_READDMA_EXT:
    case WIN_READDMA:
    case WIN_READDMA_ONCE:
    case WIN_WRITEDMA_EXT:
    case WIN_WRITEDMA:
    case WIN_WRITEDMA_ONCE:
        not_implemented(val);
        goto abort_cmd;
        break;

    case WIN_READ_NATIVE_MAX_EXT:
        lba48 = 1;
    case WIN_READ_NATIVE_MAX:
        ide_cmd_lba48_transform(s, lba48);
        ide_set_sector(s, s->nb_sectors - 1);
        s->status = READY_STAT | SEEK_STAT;
        break;

    case WIN_CHECKPOWERMODE1:
    case WIN_CHECKPOWERMODE2:
        s->nsector = 0xff;                      /* device active or idle */
        s->status = READY_STAT | SEEK_STAT;
        break;

    case WIN_SETFEATURES:
        switch(s->feature) {
        case FEAT_ENABLE_REVERTING_TO_DEFAULTS:
        case FEAT_DISABLE_REVERTING_TO_DEFAULTS:
        case FEAT_ENABLE_WRITE_CACHE:
        case FEAT_DISABLE_WRITE_CACHE:
        case FEAT_ENABLE_READ_LOOKAHEAD:
        case FEAT_DISABLE_READ_LOOKAHEAD:
        case FEAT_ENABLE_ADVANCED_PM:
        case FEAT_DISABLE_ADVANCED_PM:
        case FEAT_ENABLE_AUTO_ACOUSTIC_MNGMNT:
        case FEAT_DISABLE_AUTO_ACOUSTIC_MNGMNT:
            s->status = READY_STAT | SEEK_STAT;
            break;
        case FEAT_SET_TRANSFER_MODE:
            if ( (s->nsector >> 3) <= 1) { /* 0: pio default, 1: pio mode */
                /* set identify_data accordingly */
            } else { /* single word dma, mdma and udma mode */
                goto abort_cmd;
            }
            s->status = READY_STAT | SEEK_STAT;
            break;
        case FEAT_ENABLE_8BIT_DATA_TRANSFERS:
            if (IDE_debug) fprintf(stderr, "ide: enable 8-bit mode\n");
            s->do_8bit = 1;
            s->cycle   = 0;
            s->status  = READY_STAT | SEEK_STAT;
            break;
        case FEAT_DISABLE_8BIT_DATA_TRANSFERS:
            if (IDE_debug) fprintf(stderr, "ide: disable 8-bit mode\n");
            s->do_8bit = 0;
            s->status  = READY_STAT | SEEK_STAT;
            break;
        default:
            goto abort_cmd;
            break;
        }
        break;

    case WIN_FLUSH_CACHE:
    case WIN_FLUSH_CACHE_EXT:
        fflush(s->file);
        break;

    case WIN_STANDBY:
    case WIN_STANDBY2:
    case WIN_STANDBYNOW1:
    case WIN_STANDBYNOW2:
    case WIN_IDLEIMMEDIATE:
    case CFA_IDLEIMMEDIATE:
    case WIN_SETIDLE1:
    case WIN_SETIDLE2:
    case WIN_SLEEPNOW1:
    case WIN_SLEEPNOW2:
        s->status = READY_STAT;
        break;

    case WIN_SEEK:
        if(s->is_cdrom) goto abort_cmd;
        /* XXX: Check that seek is within bounds and return error if not */
        s->status = READY_STAT | SEEK_STAT;
        break;

    /* ATAPI Commands SKIPPED */

    /* CF-ATA commands */
    case CFA_REQ_EXT_ERROR_CODE:
        if (!s->is_cf) goto abort_cmd;
        s->error  = 0x09;    /* miscellaneous error, MARK_ERR | MCR_ERR */
        s->status = READY_STAT | SEEK_STAT;
        break;

    case CFA_ERASE_SECTORS:
    case CFA_WEAR_LEVEL:
        if (!s->is_cf)                goto abort_cmd;
        if (val == CFA_WEAR_LEVEL)    s->nsector = 0;
        if (val == CFA_ERASE_SECTORS) s->media_changed = 1;
        s->error  = 0;
        s->status = READY_STAT | SEEK_STAT;
        break;

    case CFA_TRANSLATE_SECTOR:
        if (!s->is_cf) goto abort_cmd;
        s->error  = 0;
        s->status = READY_STAT | SEEK_STAT;
        memset(s->io_buffer, 0, 0x200);
        s->io_buffer[0x00] = s->hcyl;                       /* Cyl MSB */
        s->io_buffer[0x01] = s->lcyl;                       /* Cyl LSB */
        s->io_buffer[0x02] = s->select;                     /* Head */
        s->io_buffer[0x03] = s->sector;                     /* Sector */
        s->io_buffer[0x04] = ide_get_sector(s) >> 16;       /* LBA MSB */
        s->io_buffer[0x05] = ide_get_sector(s) >> 8;        /* LBA */
        s->io_buffer[0x06] = ide_get_sector(s) >> 0;        /* LBA LSB */
        s->io_buffer[0x13] = 0x00;                          /* Erase flag */
        s->io_buffer[0x18] = 0x00;                          /* Hot count */
        s->io_buffer[0x19] = 0x00;                          /* Hot count */
        s->io_buffer[0x1a] = 0x01;                          /* Hot count */
        ide_transfer_start(s, s->io_buffer, 0x200, ide_transfer_stop);
        break;

    case CFA_ACCESS_METADATA_STORAGE:
        if (!s->is_cf) goto abort_cmd;
        not_implemented(val);   /* FIXME: ... not yet */
        goto abort_cmd;
        break;

    case IBM_SENSE_CONDITION:
        if (!s->is_cf) goto abort_cmd;
        switch (s->feature) {
            case 0x01:  /* sense temperature in device */
                s->nsector = 0x50;      /* +20 C */
                break;
            default:
                goto abort_cmd;
        }
        s->status = READY_STAT | SEEK_STAT;
        break;

    case WIN_SMART:
        not_implemented(val);   /* FIXME: ... not yet */
        goto abort_cmd;
        break;

    default:
    abort_cmd:
        ide_abort_command(s);
        break;
    }
}

static void ide_clear_hob(struct ide_device *s) {
    s->select &= ~(1<<7);
}

static void ide_ioport_write(struct ide_device *s, uint16_t addr, uint8_t val){
    if (IDE_debug)
        fprintf(stderr, "ide: put: addr: %04x, byte: %02x\n", addr, val);

    addr &= 7;

    /* ignore writes to command block while busy */
    if (addr != 7 && s->bus_status & (BUSY_STAT|DRQ_STAT))
        return;

    switch(addr) {
    case 0: /* bottom ide layer does nothing here */ break;
    case 1:
        ide_clear_hob(s);
        s->hob_feature = s->feature;
        s->feature = val;
        break;
    case 2:
        ide_clear_hob(s);
        s->hob_nsector = s->nsector;
        s->nsector = val;
        break;
    case 3:
        ide_clear_hob(s);
        s->hob_sector = s->sector;
        s->sector = val;
        break;
    case 4:
        ide_clear_hob(s);
        s->hob_lcyl = s->lcyl;
        s->lcyl = val;
        break;
    case 5:
        ide_clear_hob(s);
        s->hob_hcyl = s->hcyl;
        s->hcyl = val;
        break;
    case 6:
        /* FIXME: HOB readback uses bit 7 */
        s->select = (val & ~0x10) | 0xa0;
        s->bus_unit = (val>>4)&1;
        break;
    default:
    case 7:
        if (IDE_debug) fprintf(stderr, "\tIDE: CMD=%02x\n", val);

        ide_transfer_stop(s);

/*
        if ( (s->status & (BUSY_STAT|DRQ_STAT)) && val != WIN_DEVICE_RESET)
            break;
*/

        ide_command(s, val);

        break;
    }
}

static uint16_t ide_data_readw(struct ide_device *s, int addr) {
    uint8_t *p;
    uint16_t ret;

    /* PIO data access only when DRQ bit is set */
    if (!(s->status & DRQ_STAT)) return 0;

    /* LE16 */
    p           = s->data_ptr;
    ret         = p[0];
    ret        |= p[1] << 8;
    p          += 2;
    s->data_ptr = p;

    if (IDE_debug) {
        fprintf(stderr, "data_readw: %d, %04x (count: %d)\n", addr, ret, count);
        count++;
        count &= 0xff;
    }

    if (p >= s->data_end) s->end_transfer_func(s);

    return ret;
}

static void ide_data_writew(struct ide_device *s, int addr, uint16_t val) {
    uint8_t *p;

    if (IDE_debug) fprintf(stderr, "data_writew: %d, %04x\n", addr, val);

    /* PIO data access only when DRQ bit is set */
    if (!(s->status & DRQ_STAT)) return;

    /* LE16 */
    p           = s->data_ptr;
    p[0]        = val & 0xff;
    p[1]        = val >> 8;
    p          += 2;
    s->data_ptr = p;

    if (p >= s->data_end) s->end_transfer_func(s);
}

static uint8_t mmio_ide_read(struct ide_device *s, int addr) {
    uint16_t ret;   /* will be cast at return */

    addr &= 15;

    if (addr == 0) {
        if (!s->do_8bit) {
            ret = ide_data_readw(s, 0);
            s->upperhalf[0] = ret & 0xff00;
            return ret;
        }
        if (!s->cycle) {
            s->data = ide_data_readw(s, 0);
            ret = s->data & 0xff;
        } else {
            ret = s->data >> 8;
        }
        s->cycle = !s->cycle;
        return ret;
    } else if (addr >= 8) {
        return s->upperhalf[addr-8] >> 8;
    } else {
        ret = ide_ioport_read(s, addr);
        s->upperhalf[addr] = ret & 0xff00;
        return ret;
    }
}

static void mmio_ide_write(struct ide_device *s, int addr, uint8_t val) {
    addr &= 15;

    if (addr == 0) {
        if (!s->do_8bit) {
            ide_data_writew(s, 0, s->upperhalf[0] | val);
            return;
        }
        if (!s->cycle) {
            s->data = val & 0xff;
        } else {
            ide_data_writew(s, 0, s->data | (val << 8));
        }
        s->cycle = !s->cycle;
    } else if (addr >= 8) {
        s->upperhalf[addr-8] = val << 8;
    } else {
        ide_ioport_write(s, addr, s->upperhalf[addr] | val);
    }
}

void IDE_PutByte(uint16_t addr, uint8_t val) {
    struct ide_device *s = &device;
    mmio_ide_write(s, addr, val);
}

uint8_t IDE_GetByte(uint16_t addr) {
    struct ide_device *s = &device;
    return mmio_ide_read(s, addr);
}

int IDE_Initialise(int *argc, char *argv[]) {
    int i, j, ret = TRUE;
    char *filename = NULL;

    if (IDE_debug)
        fprintf(stderr, "ide: init\n");

    for (i = j = 1; i < *argc; i++) {
        int available = i + 1 < *argc;

        if (!strcmp(argv[i], "-ide"  )) {
            if (!available) {
                Log_print("Missing argument for '%s'", argv[i]);
                return FALSE;
            }
            filename = Util_strdup(argv[++i]);
        } else if (!strcmp(argv[i], "-ide_debug")) {
            IDE_debug = 1;
        } else if (!strcmp(argv[i], "-ide_cf")) {
            device.is_cf = 1;
        } else {
             if (!strcmp(argv[i], "-help")) {
                 Log_print("\t-ide <file>      Enable IDE emulation");
                 Log_print("\t-ide_debug       Enable IDE Debug Output");
                 Log_print("\t-ide_cf          Enable CF emulation");
             }
             argv[j++] = argv[i];
        }
    }
    *argc = j;

    if (filename) {
        IDE_enabled = ret = ide_init_drive(&device, filename);
        free(filename);
    }

    return ret;
}
