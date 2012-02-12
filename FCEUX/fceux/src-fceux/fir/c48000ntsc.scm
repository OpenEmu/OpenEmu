(title "48000 NTSC")

(verbose #t)

(cosine-symmetry)
(filter-length 1024)
(sampling-frequency 1789772.727272727272)
(limit-= (band 0 17000) 1)
(limit-= (band 24000 894886.363636) 0 .5)
(output-file "c48000ntsc.coef")
(plot-file "c48000ntsc.plot")
(go)
