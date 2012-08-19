#!/bin/bash

if [ -f ../../generated/driverlist.h ]; then
   echo "Generated files already built, deleting old files first..."
   rm -rf ../../generated
fi

cd ../..
if [ -d ../../generated ]; then
   echo "Directory 'generated' already exists, skipping creation...\n"
else
   mkdir -p generated
fi

#generate gamelist.txt and generated/driverlist.h
echo ""
echo "generate_driverlist.sh: Generate all files from scripts directory"
echo ""
echo "1) gamelist.pl"
perl dep/scripts/gamelist.pl -o generated/driverlist.h -l gamelist.txt \
burn/drv/capcom \
burn/drv/cave \
burn/drv/cps3 \
burn/drv/dataeast \
burn/drv/galaxian \
burn/drv/irem \
burn/drv/konami \
burn/drv/megadrive \
burn/drv/neogeo \
burn/drv/pre90s \
burn/drv/pst90s \
burn/drv/pce \
burn/drv/pgm \
burn/drv/psikyo \
burn/drv/sega \
burn/drv/snes \
burn/drv/taito \
burn/drv/toaplan

#generate generated/neo_sprite_func.h and generated/neo_sprite_func_table.h
echo ""
echo "2) neo_sprite_func.pl"
perl dep/scripts/neo_sprite_func.pl -o generated/neo_sprite_func.h

#generate generated/psikyo_tile_func.h and generated/psikyo_tile_func_table.h
echo ""
echo "3) psikyo_tile_func.pl"
perl dep/scripts/psikyo_tile_func.pl -o generated/psikyo_tile_func.h

#generate generated/cave_sprite_func.h and generated/cave_sprite_func_table.h
echo ""
echo "4) cave_sprite_func.pl"
perl dep/scripts/cave_sprite_func.pl -o generated/cave_sprite_func.h

#generate generated/cave_tile_func.h and generated/cave_tile_func_table.h
echo ""
echo "5) cave_tile_func.pl"
perl dep/scripts/cave_tile_func.pl -o generated/cave_tile_func.h

#generate generated/toa_gp9001_func.h and generated/toa_gp9001_func_table.h
echo ""
echo "6) toa_gp9001_func.pl"
perl dep/scripts/toa_gp9001_func.pl -o generated/toa_gp9001_func.h

echo ""
echo "7) pgm_sprite_create.cpp"
gcc -o pgm_sprite_create burn/drv/pgm/pgm_sprite_create.cpp
./pgm_sprite_create > burn/drv/pgm/pgm_sprite.h

#compile m68kmakeecho 
echo ""
echo "8) compile m68kmake"
gcc -o m68kmake cpu/m68k/m68kmake.c

#create m68kops.h with m68kmake
echo ""
echo "9) Create m68kops.h with m68kmake"
./m68kmake cpu/m68k/ cpu/m68k/m68k_in.c

#compile ctv_make
echo ""
echo "10) compile ctvmake"
gcc -o  ctvmake burn/drv/capcom/ctv_make.cpp

#create ctv.h
echo ""
echo "11) Create ctv.h with ctvmake"
./ctvmake > generated/ctv.h
