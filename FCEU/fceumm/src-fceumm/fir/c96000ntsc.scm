(title "96000 NTSC")

(verbose #t)

(cosine-symmetry)
(filter-length 1024)
(sampling-frequency 1789772.727272727272)
(limit-= (band 0 36850) 1)
(limit-= (band 48000 894886.363636) 0 .5)
(output-file "c96000ntsc.coef")
(plot-file "c96000ntsc.plot")
(go)
