(title "96000 PAL")

(verbose #t)

(cosine-symmetry)
(filter-length 1024)
(sampling-frequency 1662607.125)
(limit-= (band 0 33400) 1)
(limit-= (band 48000 831303.5625) 0 .5)
(output-file "c96000pal.coef")
(plot-file "c96000pal.plot")
(go)
