from math import sqrt

C_AUDAY = 173.1446326846693

def aberration(position, velocity, lighttime):
    """Corrects a relative position vector for aberration of light."""

    pos, ve = position, velocity

    if lighttime:
        p1mag = lighttime * C_AUDAY
    else:
        p1mag = sqrt(pos[0] * pos[0] + pos[1] * pos[1] + pos[2] * pos[2])
        lighttime = p1mag / C_AUDAY

    vemag = sqrt(ve[0] * ve[0] + ve[1] * ve[1] + ve[2] * ve[2])
    beta = vemag / C_AUDAY
    dot = pos[0] * ve[0] + pos[1] * ve[1] + pos[2] * ve[2]

    cosd = dot / (p1mag * vemag)
    gammai = sqrt(1.0 - beta * beta)
    p = beta * cosd
    q = (1.0 + p / (1.0 + gammai)) * lighttime
    r = 1.0 + p

    return [(gammai * pos[0] + q * ve[0]) / r,
            (gammai * pos[1] + q * ve[1]) / r,
            (gammai * pos[2] + q * ve[2]) / r]
