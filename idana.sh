#!/bin/sh
cp -r ~/one/6_program_okamoto/1_ImpDet_ANA/1_info/ .
mkdir 2_txt
mkdir 3_pdf
mkdir 4_gif

root -l -b -q ~/one/6_program_okamoto/1_ImpDet_ANA/ImpDet_pedestal.cpp+ | tee pedestal_log.txt
root -l ~/one/6_program_okamoto/1_ImpDet_ANA/ImpDet_ANA.cpp+ | tee ANA_log.txt

