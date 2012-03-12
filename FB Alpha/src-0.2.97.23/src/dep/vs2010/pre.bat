@echo off
cd src/depend
mkdir generated
cd scripts
echo running perl scripts
perl cave_sprite_func.pl -o ../generated/cave_sprite_func.h
perl license2rtf.pl -o ../generated/license.rtf ../../license.txt
perl cave_tile_func.pl -o ../generated/cave_tile_func.h
perl neo_sprite_func.pl -o ../generated/neo_sprite_func.h
perl psikyo_tile_func.pl -o ../generated/psikyo_tile_func.h
perl toa_gp9001_func.pl -o ../generated/toa_gp9001_func.h
perl gamelist.pl -o ../generated/driverlist.h -l ../../gamelist.txt ../../burn/drivers/capcom ../../burn/drivers/cave ../../burn/drivers/cps3 ../../burn/drivers/galaxian ../../burn/drivers/irem ../../burn/drivers/konami ../../burn/drivers/megadrive ../../burn/drivers/misc_post90s ../../burn/drivers/misc_pre90s ../../burn/drivers/neogeo ../../burn/drivers/pgm ../../burn/drivers/psikyo ../../burn/drivers/sega ../../burn/drivers/taito ../../burn/drivers/toaplan ../../burn/drivers/snes
echo building buildinfo
%1/bin/cl" build_details.cpp
build_details > ../generated/build_details.h
del build_details.exe
del build_details.obj
echo building ctv
cd ..
cd ..
cd burn
cd drivers
cd capcom
%1/bin/cl" ctv_make.cpp
ctv_make > ../../../depend/generated/ctv.h
del ctv_make.exe
del ctv_make.obj

echo building 68k generator
cd ..
cd ..
cd ..
cd cpu
cd m68k
%1/bin/cl" /DINLINE="__inline static" m68kmake.c
m68kmake ../../depend/generated/ m68k_in.c
del m68kmake.exe
del m68kmake.obj

