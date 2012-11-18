#ifdef NALL_MOSAIC_INTERNAL_HPP

namespace nall {
namespace mosaic {

struct parser {
  image canvas;

  //export from bitstream to canvas
  inline void load(bitstream &stream, uint64_t offset, context &ctx, unsigned width, unsigned height) {
    canvas.allocate(width, height);
    canvas.clear(ctx.paddingColor);
    parse(1, stream, offset, ctx, width, height);
  }

  //import from canvas to bitstream
  inline bool save(bitstream &stream, uint64_t offset, context &ctx) {
    if(stream.readonly) return false;
    parse(0, stream, offset, ctx, canvas.width, canvas.height);
    return true;
  }

  inline parser() : canvas(0, 32, 0u, 255u << 16, 255u << 8, 255u << 0) {
  }

private:
  inline uint32_t read(unsigned x, unsigned y) const {
    unsigned addr = y * canvas.width + x;
    if(addr >= canvas.width * canvas.height) return 0u;
    uint32_t *buffer = (uint32_t*)canvas.data;
    return buffer[addr];
  }

  inline void write(unsigned x, unsigned y, uint32_t data) {
    unsigned addr = y * canvas.width + x;
    if(addr >= canvas.width * canvas.height) return;
    uint32_t *buffer = (uint32_t*)canvas.data;
    buffer[addr] = data;
  }

  inline void parse(bool load, bitstream &stream, uint64_t offset, context &ctx, unsigned width, unsigned height) {
    stream.endian = ctx.endian;
    unsigned canvasWidth = width / (ctx.mosaicWidth * ctx.tileWidth * ctx.blockWidth + ctx.paddingWidth);
    unsigned canvasHeight = height / (ctx.mosaicHeight * ctx.tileHeight * ctx.blockHeight + ctx.paddingHeight);
    unsigned bitsPerBlock = ctx.depth * ctx.blockWidth * ctx.blockHeight;

    unsigned objectOffset = 0;
    for(unsigned objectY = 0; objectY < canvasHeight; objectY++) {
      for(unsigned objectX = 0; objectX < canvasWidth; objectX++) {
        if(objectOffset >= ctx.count && ctx.count > 0) break;
        unsigned objectIX = objectX * ctx.objectWidth();
        unsigned objectIY = objectY * ctx.objectHeight();
        objectOffset++;

        unsigned mosaicOffset = 0;
        for(unsigned mosaicY = 0; mosaicY < ctx.mosaicHeight; mosaicY++) {
          for(unsigned mosaicX = 0; mosaicX < ctx.mosaicWidth; mosaicX++) {
            unsigned mosaicData = ctx.mosaic(mosaicOffset, mosaicOffset);
            unsigned mosaicIX = (mosaicData % ctx.mosaicWidth) * (ctx.tileWidth * ctx.blockWidth);
            unsigned mosaicIY = (mosaicData / ctx.mosaicWidth) * (ctx.tileHeight * ctx.blockHeight);
            mosaicOffset++;

            unsigned tileOffset = 0;
            for(unsigned tileY = 0; tileY < ctx.tileHeight; tileY++) {
              for(unsigned tileX = 0; tileX < ctx.tileWidth; tileX++) {
                unsigned tileData = ctx.tile(tileOffset, tileOffset);
                unsigned tileIX = (tileData % ctx.tileWidth) * ctx.blockWidth;
                unsigned tileIY = (tileData / ctx.tileWidth) * ctx.blockHeight;
                tileOffset++;

                unsigned blockOffset = 0;
                for(unsigned blockY = 0; blockY < ctx.blockHeight; blockY++) {
                  for(unsigned blockX = 0; blockX < ctx.blockWidth; blockX++) {
                    if(load) {
                      unsigned palette = 0;
                      for(unsigned n = 0; n < ctx.depth; n++) {
                        unsigned index = blockOffset++;
                        if(ctx.order == 1) index = (index % ctx.depth) * ctx.blockWidth * ctx.blockHeight + (index / ctx.depth);
                        palette |= stream.read(offset + ctx.block(index, index)) << n;
                      }

                      write(
                        objectIX + mosaicIX + tileIX + blockX,
                        objectIY + mosaicIY + tileIY + blockY,
                        ctx.palette(palette, palette)
                      );
                    } else /* save */ {
                      uint32_t palette = read(
                        objectIX + mosaicIX + tileIX + blockX,
                        objectIY + mosaicIY + tileIY + blockY
                      );

                      for(unsigned n = 0; n < ctx.depth; n++) {
                        unsigned index = blockOffset++;
                        if(ctx.order == 1) index = (index % ctx.depth) * ctx.blockWidth * ctx.blockHeight + (index / ctx.depth);
                        stream.write(offset + ctx.block(index, index), palette & 1);
                        palette >>= 1;
                      }
                    }
                  }  //blockX
                }  //blockY

                offset += ctx.blockStride;
              }  //tileX

              offset += ctx.blockOffset;
            }  //tileY

            offset += ctx.tileStride;
          }  //mosaicX

          offset += ctx.tileOffset;
        }  //mosaicY

        offset += ctx.mosaicStride;
      }  //objectX

      offset += ctx.mosaicOffset;
    }  //objectY
  }
};

}
}

#endif
