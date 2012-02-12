(title "44100 PAL")

(verbose #t)

(cosine-symmetry)
(filter-length 1024)
(sampling-frequency 1662607.125)
(limit-= (band 0 15500) 1)
(limit-= (band 22050 831303.5625) 0 .5)
(output-file "c44100pal.coef")
(plot-file "c44100pal.plot")
(go)
