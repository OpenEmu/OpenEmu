(title "48000 PAL")

(verbose #t)

(cosine-symmetry)
(filter-length 1024)
(sampling-frequency 1662607.125)
(limit-= (band 0 17500) 1)
(limit-= (band 24000 831303.5625) 0 .5)
(output-file "c48000pal.coef")
(plot-file "c48000pal.plot")
(go)
