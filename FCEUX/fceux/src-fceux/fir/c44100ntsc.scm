(title "44100 NTSC")

(verbose #t)

(cosine-symmetry)
(filter-length 1024)
(sampling-frequency 1789772.727272727272)
(limit-= (band 0 15000) 1)
(limit-= (band 22050 894886.363636) 0 .5)
(output-file "c44100ntsc.coef")
(plot-file "c44100ntsc.plot")
(go)
