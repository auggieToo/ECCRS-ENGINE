// jotter-notes.typ — handwritten-notebook styling for ordinary Typst documents.
// v2: hand-drawn arrows, circling, corrections, plots, tables, taped photos,
//     flow diagrams. Everything wobbles deterministically (seeded), so
//     recompiles are stable.
//
// Usage:  #import "jotter-notes.typ": *
//         #show: jotter-notes.with(title: [My notes])

#let ink = state("jotter-ink", rgb("#28356b"))
#let accent = state("jotter-accent", rgb("#d94f4f"))

// ================================================================ internals
// All geometry is done in float "pt space", converted to lengths at the end.

#let _wob(t, seed, amp) = (
  calc.sin(t * 6.7 + seed * 1.7) * amp
    + calc.sin(t * 17.3 + seed * 3.1) * amp * 0.45
    + calc.sin(t * 31.9 + seed * 0.7) * amp * 0.2
)

#let _rand(seed) = calc.fract(calc.sin(seed * 12.9898) * 43758.5453)

#let _pts(fpts) = fpts.map(p => (p.at(0) * 1pt, p.at(1) * 1pt))

#let _stroke(color, thickness) = (
  paint: color, thickness: thickness, cap: "round", join: "round",
)

// polyline through float points, jittered perpendicular-ish
#let _hand(fpts, color, thickness, amp, seed) = {
  let n = fpts.len()
  let out = ()
  for (i, p) in fpts.enumerate() {
    let t = i / calc.max(n - 1, 1)
    let w = _wob(t, seed, amp)
    // jitter perpendicular to local direction
    let j = calc.max(i, 1)
    let dx = fpts.at(j).at(0) - fpts.at(j - 1).at(0)
    let dy = fpts.at(j).at(1) - fpts.at(j - 1).at(1)
    let len = calc.max(calc.sqrt(dx * dx + dy * dy), 0.001)
    out.push((p.at(0) - dy / len * w, p.at(1) + dx / len * w))
  }
  curve(
    stroke: _stroke(color, thickness),
    curve.move(_pts(out).at(0)),
    .._pts(out).slice(1).map(p => curve.line(p)),
  )
}

// ============================================================ line & shapes

// hand-drawn straight-ish line. style: "line" | "wave" | "double"
#let sketch-line(
  len, color: black, thickness: 1.1pt, amp: 0.5pt, seed: 0, style: "line",
) = {
  let L = len.pt()
  let n = 16
  let base = range(n + 1).map(i => (i / n * L, 0.0))
  if style == "wave" {
    base = range(n + 1).map(i => (
      i / n * L,
      calc.sin(i / n * L / 4.2) * 1.6,
    ))
  }
  _hand(base, color, thickness, amp.pt(), seed)
  if style == "double" {
    place(dy: 2.4pt, _hand(base, color, thickness * 0.8, amp.pt(), seed + 11))
  }
}

#let sketch-rect(
  w, h, color: black, fill: none, thickness: 1.1pt, amp: 0.6pt, seed: 3,
) = {
  let W = w.pt()
  let H = h.pt()
  let side(a, b, s) = {
    let n = 10
    range(n + 1).map(i => {
      let t = i / n
      (a.at(0) + (b.at(0) - a.at(0)) * t, a.at(1) + (b.at(1) - a.at(1)) * t)
    })
  }
  // slight corner overshoot: start a touch before the corner, like a real pen
  let pts = (
    ..side((-1.0, 0.4), (W, 0.0), seed),
    ..side((W, 0.0), (W, H), seed + 1),
    ..side((W, H), (0.0, H), seed + 2),
    ..side((0.0, H), (0.0, -1.2), seed + 3),
  )
  if fill != none {
    place(curve(
      fill: fill, stroke: none,
      curve.move((0pt, 0pt)), curve.line((w, 0pt)),
      curve.line((w, h)), curve.line((0pt, h)), curve.close(),
    ))
  }
  _hand(pts, color, thickness, 0.55, seed)
}

// hand-drawn arrow. dx/dy: displacement. bend: sideways bow of the shaft.
#let harrow(
  dx, dy: 0pt, color: auto, thickness: 1.2pt, bend: 8pt, seed: 0, head: 7pt,
) = context {
  let col = if color == auto { accent.get() } else { color }
  let X = dx.pt(); let Y = dy.pt(); let B = bend.pt(); let Hd = head.pt()
  let L = calc.max(calc.sqrt(X * X + Y * Y), 0.001)
  let px = -Y / L; let py = X / L        // unit perpendicular
  let n = 14
  let shaft = range(n + 1).map(i => {
    let t = i / n
    let bow = calc.sin(t * calc.pi) * B
    (X * t + px * bow, Y * t + py * bow)
  })
  // direction at the tip
  let a = shaft.at(n - 1); let b = shaft.at(n)
  let ang = calc.atan2(b.at(0) - a.at(0), b.at(1) - a.at(1))
  let wing(off) = (
    (b.at(0), b.at(1)),
    (
      b.at(0) + Hd * calc.cos(ang + off),
      b.at(1) + Hd * calc.sin(ang + off),
    ),
  )
  box(width: 0pt, height: 0pt, {
    place(_hand(shaft, col, thickness, 0.6, seed))
    place(_hand(wing(152deg), col, thickness, 0.25, seed + 5))
    place(_hand(wing(-152deg), col, thickness, 0.25, seed + 6))
  })
}

// inline "→" that looks drawn; use mid-sentence: idea A #ar idea B
#let ar(color: auto, len: 1.5em) = context box(
  width: len, height: 0.62em,
  place(dy: 0.42em, harrow(len - 0.2em, bend: 1.5pt, color: color,
    thickness: 1.1pt, head: 4.5pt, seed: 17)),
)

// ================================================================= inline ink

// hand-drawn ring around inline content
#let circled(body, color: auto, seed: 2) = context {
  let col = if color == auto { accent.get() } else { color }
  let sz = measure(body)
  let rx = sz.width.pt() / 2 + 7
  let ry = sz.height.pt() / 2 + 5.5
  let cx = sz.width.pt() / 2
  let cy = sz.height.pt() / 2
  let n = 26
  let pts = range(n + 1).map(i => {
    let th = -0.5 + i / n * (2 * calc.pi + 0.9)   // overlap the start
    let rj = 1 + _wob(i / n, seed, 0.035)
    (cx + rx * rj * calc.cos(th), cy + ry * rj * calc.sin(th))
  })
  box({
    body
    place(dx: -sz.width, dy: -sz.height, _hand(pts, col, 1.2pt, 0.0, seed))
  })
}

#let hl(body, color: rgb("#ffe680")) = highlight(
  fill: color.transparentize(25%), extent: 1.5pt, radius: 3pt, body,
)

// underline styles: "line" (default), "wave", "double"
#let scribble(body, color: auto, style: "line") = context {
  let col = if color == auto { accent.get() } else { color }
  let w = measure(body).width
  box({
    body
    place(dx: -w, dy: 0.28em, sketch-line(w, color: col, seed: 5, style: style))
  })
}

// crossed-out
#let strike-hand(body, color: auto) = context {
  let col = if color == auto { accent.get() } else { color }
  let sz = measure(body)
  box({
    body
    place(dx: -sz.width - 2pt, dy: -sz.height * 0.45,
      sketch-line(sz.width + 4pt, color: col, thickness: 1.3pt, amp: 1pt, seed: 21))
  })
}

// crossed-out old value with the fix written above it
#let correction(old, new, color: auto) = context {
  let col = if color == auto { accent.get() } else { color }
  box({
    strike-hand(old, color: col)
    place(dx: -measure(old).width * 0.8, dy: -1.35em,
      rotate(-3deg, text(size: 0.78em, fill: col, new)))
  })
}

// text with an arrow shooting into the right margin, note waiting there
#let pointer(body, note: none, color: auto) = context {
  let col = if color == auto { accent.get() } else { color }
  body
  box(width: 0pt, height: 0pt, place(dx: 3pt, dy: -0.7em,
    harrow(1.1cm, dy: -0.45cm, bend: -6pt, color: col, thickness: 1pt,
      head: 5pt, seed: 9)))
  place(right, dx: 100% + 0.7cm, dy: -1.9em, box(width: 3.1cm, {
    set text(size: 0.78em, fill: col)
    set par(leading: 0.55em)
    rotate(-1.5deg, note)
  }))
}

#let marginnote(body, dx: 0.7cm, color: auto, angle: -1.5deg) = context {
  let col = if color == auto { accent.get() } else { color }
  place(right, dx: dx + 100%, dy: -0.35em, box(width: 3.1cm, {
    set text(size: 0.78em, fill: col)
    set par(leading: 0.55em)
    rotate(angle, body)
  }))
}

// ================================================================== blocks

#let sketch-box(
  body, color: auto, fill: none, inset: 10pt, seed: 3, width: 100%,
  tilt: 0deg,
) = context {
  let col = if color == auto { ink.get() } else { color }
  layout(size => {
    let w = size.width * (width / 100%)
    let inner = block(width: w - 2 * inset, body)
    let h = measure(inner).height + 2 * inset
    rotate(tilt, reflow: true,
      block(width: w, height: h, breakable: false, {
        place(top + left, sketch-rect(w, h, color: col, fill: fill, seed: seed))
        place(top + left, dx: inset, dy: inset, inner)
      }))
  })
}

// auto-sized boxed content (inline-block), for flow diagrams etc.
#let bubble(body, color: auto, fill: none, inset: 8pt, seed: 6) = context {
  let col = if color == auto { ink.get() } else { color }
  let sz = measure(body)
  box(width: sz.width + 2 * inset, height: sz.height + 2 * inset, {
    place(top + left, sketch-rect(sz.width + 2 * inset, sz.height + 2 * inset,
      color: col, fill: fill, seed: seed))
    place(top + left, dx: inset, dy: inset, body)
  })
}

// boxes chained with arrows: #flow[parse][check][emit]
#let flow(dir: ltr, gap: 1.3cm, color: auto, fill: none, ..steps) = context {
  let col = if color == auto { ink.get() } else { color }
  let items = steps.pos()
  let cells = ()
  for (i, s) in items.enumerate() {
    cells.push(bubble(s, color: col, fill: fill, seed: 6 + i * 3))
    if i < items.len() - 1 {
      if dir == ltr {
        cells.push(box(width: gap, height: 8pt, place(dy: 4pt, dx: 2pt,
          harrow(gap - 6pt, bend: if calc.even(i) { 5pt } else { -5pt },
            seed: i * 7))))
      } else {
        cells.push(box(width: 8pt, height: gap, place(dx: 4pt, dy: 2pt,
          harrow(0pt, dy: gap - 6pt, bend: if calc.even(i) { 6pt } else { -6pt },
            seed: i * 7))))
      }
    }
  }
  if dir == ltr {
    grid(columns: cells.len(), align: horizon, column-gutter: 4pt, ..cells)
  } else {
    grid(rows: cells.len(), align: center, row-gutter: 4pt, ..cells)
  }
}

#let sticky(body, color: rgb("#ffe680"), angle: auto, width: 5cm, seed: 1) = {
  let a = if angle == auto { (_rand(seed) * 5 - 2.5) * 1deg } else { angle }
  rotate(a, reflow: true, block(
    width: width, fill: color, inset: 10pt,
    radius: (bottom-right: 10pt), stroke: none,
    { set text(size: 0.9em); body },
  ))
}

#let tape(width: 2.4cm, angle: -7deg) = rotate(angle, box(
  width: width, height: 0.75cm,
  fill: rgb("#efe4c0").transparentize(35%),
  stroke: (left: 0.4pt + rgb("#d8c894"), right: 0.4pt + rgb("#d8c894")),
))

// taped-in "polaroid": #photo(image("x.png"), caption: [...])
#let photo(
  content, caption: none, angle: auto, width: 7cm, seed: 4, tape-it: true,
) = context {
  let a = if angle == auto { (_rand(seed) * 4 - 2) * 1deg } else { angle }
  let inner = block(width: width - 16pt, content)
  let h = measure(inner).height
  box(rotate(a, reflow: true, {
    block(width: width, fill: white, inset: (x: 8pt, top: 8pt, bottom: 8pt),
      stroke: 0.5pt + luma(78%), breakable: false, {
        inner
        if caption != none {
          v(5pt)
          align(center, text(size: 0.78em, fill: accent.get(),
            style: "italic", caption))
        }
      })
    if tape-it {
      place(top + left, dx: -8pt, dy: -7pt, tape(angle: -35deg, width: 1.7cm))
      place(top + right, dx: 8pt, dy: -7pt, tape(angle: 35deg, width: 1.7cm))
    }
  }))
}

// ============================================================ hand tables

// #stable(columns: 3, [h1],[h2],[h3], [a],[b],[c], ...)
// columns: int (equal) or array of weights, e.g. (2, 1, 1)
#let stable(
  columns: 2, header: true, inset: 7pt, color: auto, width: 100%,
  cell-align: left, seed: 5, ..data,
) = context {
  let col = if color == auto { ink.get() } else { color }
  let cells = data.pos()
  layout(size => {
    let W = size.width * (width / 100%)
    let weights = if type(columns) == int {
      range(columns).map(_ => 1.0)
    } else { columns.map(w => float(w)) }
    let nc = weights.len()
    let tot = weights.sum()
    let colw = weights.map(w => W * w / tot)
    // chunk into rows
    let rows = ()
    let i = 0
    while i < cells.len() {
      rows.push(cells.slice(i, calc.min(i + nc, cells.len())))
      i += nc
    }
    // measure row heights
    let rowh = rows.map(r => {
      let hmax = 0pt
      for (j, c) in r.enumerate() {
        let cw = colw.at(j) - 2 * inset
        let ch = measure(block(width: cw, c)).height
        if ch > hmax { hmax = ch }
      }
      hmax + 2 * inset
    })
    let H = rowh.sum()
    block(width: W, height: H, breakable: false, {
      // header wash
      if header {
        place(top + left, curve(fill: rgb("#fff3bf").transparentize(30%),
          stroke: none,
          curve.move((2pt, 2pt)), curve.line((W - 2pt, 2pt)),
          curve.line((W - 2pt, rowh.at(0))), curve.line((2pt, rowh.at(0))),
          curve.close()))
      }
      // cells
      let y = 0pt
      for (ri, r) in rows.enumerate() {
        let x = 0pt
        for (ci, c) in r.enumerate() {
          let cw = colw.at(ci) - 2 * inset
          let styled = if header and ri == 0 {
            text(weight: "bold", c)
          } else { c }
          place(top + left, dx: x + inset, dy: y + inset,
            block(width: cw, align(cell-align, styled)))
          x += colw.at(ci)
        }
        y += rowh.at(ri)
      }
      // hand-drawn grid
      place(top + left, sketch-rect(W, H, color: col, seed: seed))
      let y2 = 0pt
      for (ri, rh) in rowh.enumerate() {
        y2 += rh
        if ri < rows.len() - 1 {
          place(top + left, dy: y2,
            sketch-line(W, color: col, thickness: 0.9pt, seed: seed + ri + 1))
        }
      }
      let x2 = 0pt
      for (ci, cw) in colw.enumerate() {
        x2 += cw
        if ci < nc - 1 {
          place(top + left, dx: x2, dy: 0pt, rotate(90deg, origin: top + left,
            sketch-line(H, color: col, thickness: 0.9pt, seed: seed + 40 + ci)))
        }
      }
    })
  })
}

#let hs(s) = text(fill: rgb("#ff0600"))

// ================================================================== graphs

// hand-drawn plot.
// #graph(domain: (-3, 3), (x => x*x, [x²]), (x => 2*x + 1, [2x+1]))
// series: function, or (function, label), or (function, label, color)
#let graph(
  domain: (0, 1), range-y: auto, width: 8cm, height: 5.5cm,
  samples: 48, grid: false, color: auto, seed: 8,
  points: (), point-label: none,
  xlabel: none, ylabel: none,
  ..series,
) = context {
  let axis-col = if color == auto { ink.get() } else { color }
  let palette = (accent.get(), rgb("#2a7fbf"), rgb("#3d8f5f"), rgb("#b06ab3"))
  let fns = series.pos().enumerate().map(((i, s)) => {
    if type(s) == function { (fn: s, label: none, col: palette.at(calc.rem(i, 4))) }
    else if s.len() == 2 { (fn: s.at(0), label: s.at(1), col: palette.at(calc.rem(i, 4))) }
    else { (fn: s.at(0), label: s.at(1), col: s.at(2)) }
  })
  let (x0, x1) = domain
  let curves = fns.map(f => {
    let ys = range(samples + 1).map(i => {
      let x = x0 + (x1 - x0) * i / samples
      (x, (f.fn)(x))
    })
    (..f, pts: ys)
  })
  let allys = curves.map(c => c.pts.map(p => p.at(1))).flatten()
  allys += points.map(p => p.at(1))
  let (y0, y1) = if range-y == auto {
    if allys.len() == 0 { (0, 1) } else {
      let lo = calc.min(..allys); let hi = calc.max(..allys)
      let pad = calc.max((hi - lo) * 0.08, 0.001)
      (lo - pad, hi + pad)
    }
  } else { range-y }
  let W = width.pt(); let H = height.pt()
  let mx(x) = (x - x0) / (x1 - x0) * (W - 26) + 16.0
  let my(y) = H - 18 - (y - y0) / calc.max(y1 - y0, 0.000001) * (H - 30)
  // axis positions: through zero if visible, else at edge
  let ax-y = if y0 < 0 and y1 > 0 { my(0) } else { H - 18.0 }
  let ax-x = if x0 < 0 and x1 > 0 { mx(0) } else { 16.0 }
  // tick step: nice-ish
  let nice(span) = {
    let raw = span / 4
    let p = calc.pow(10, calc.floor(calc.log(raw, base: 10)))
    let r = raw / p
    let m = if r < 1.5 { 1 } else if r < 3.5 { 2 } else if r < 7.5 { 5 } else { 10 }
    m * p
  }
  let fmt(v) = {
    let r = calc.round(v, digits: 2)
    if calc.fract(r) == 0 { str(int(r)) } else { str(r) }
  }
  block(width: width, height: height, breakable: false, {
    if grid {
      let gs = nice((x1 - x0))
      let t = calc.ceil(x0 / gs) * gs
      while t <= x1 {
        place(_hand(((mx(t), 6.0), (mx(t), H - 14.0)),
          axis-col.lighten(72%), 0.5pt, 0.3, seed + 30))
        t += gs
      }
      let gsy = nice((y1 - y0))
      let ty = calc.ceil(y0 / gsy) * gsy
      while ty <= y1 {
        place(_hand(((10.0, my(ty)), (W - 8.0, my(ty))),
          axis-col.lighten(72%), 0.5pt, 0.3, seed + 31))
        ty += gsy
      }
    }
    // axes with arrowheads
    place(dx: 6pt, dy: ax-y * 1pt,
      harrow((W - 10) * 1pt, color: axis-col, bend: 0.5pt, thickness: 1.1pt,
        head: 5.5pt, seed: seed))
    place(dx: ax-x * 1pt, dy: (H - 10) * 1pt,
      harrow(0pt, dy: (14 - H) * 1pt, color: axis-col, bend: 0.5pt,
        thickness: 1.1pt, head: 5.5pt, seed: seed + 1))
    // ticks + numbers
    let ts = nice(x1 - x0)
    let t = calc.ceil(x0 / ts) * ts
    while t <= x1 + ts * 0.01 {
      if calc.abs(t) > ts * 0.01 or not (x0 < 0 and x1 > 0) {
        place(_hand(((mx(t), ax-y - 2.5), (mx(t), ax-y + 2.5)),
          axis-col, 1pt, 0.15, seed + 3))
        place(dx: mx(t) * 1pt - 4pt, dy: ax-y * 1pt + 4pt,
          rotate((_rand(seed + t) * 4 - 2) * 1deg,
            text(size: 0.68em, fill: axis-col, fmt(t))))
      }
      t += ts
    }
    let tsy = nice(y1 - y0)
    let ty = calc.ceil(y0 / tsy) * tsy
    while ty <= y1 + tsy * 0.01 {
      if calc.abs(ty) > tsy * 0.01 or not (y0 < 0 and y1 > 0) {
        place(_hand(((ax-x - 2.5, my(ty)), (ax-x + 2.5, my(ty))),
          axis-col, 1pt, 0.15, seed + 4))
        place(dx: ax-x * 1pt - 16pt, dy: my(ty) * 1pt - 5pt,
          box(width: 13pt, align(right,
            text(size: 0.68em, fill: axis-col, fmt(ty)))))
      }
      ty += tsy
    }
    // curves (clip out-of-range segments)
    for (ci, c) in curves.enumerate() {
      let seg = ()
      let segs = ()
      for (x, y) in c.pts {
        if y >= y0 and y <= y1 { seg.push((mx(x), my(y))) }
        else { if seg.len() > 1 { segs.push(seg) }; seg = () }
      }
      if seg.len() > 1 { segs.push(seg) }
      for (si, s) in segs.enumerate() {
        place(_hand(s, c.col, 1.4pt, 0.55, seed + 10 + ci * 5 + si))
      }
      if c.label != none and segs.len() > 0 {
        let last = segs.last().last()
        place(dx: last.at(0) * 1pt + 3pt, dy: last.at(1) * 1pt - 0.6em,
          rotate(-2deg, text(size: 0.75em, fill: c.col, c.label)))
      }
    }
    // scatter: little hand x-marks
    for (pi, p) in points.enumerate() {
      let cx = mx(p.at(0)); let cy = my(p.at(1))
      place(_hand(((cx - 2.6, cy - 2.6), (cx + 2.6, cy + 2.6)),
        accent.get(), 1.3pt, 0.3, seed + 50 + pi))
      place(_hand(((cx - 2.6, cy + 2.6), (cx + 2.6, cy - 2.6)),
        accent.get(), 1.3pt, 0.3, seed + 70 + pi))
    }
    if point-label != none and points.len() > 0 {
      let lp = points.last()
      place(dx: mx(lp.at(0)) * 1pt + 5pt, dy: my(lp.at(1)) * 1pt - 1.2em,
        rotate(-2deg, text(size: 0.72em, fill: accent.get(), point-label)))
    }
    if xlabel != none {
      place(dx: (W - 8) * 1pt, dy: ax-y * 1pt - 1.5em,
        text(size: 0.75em, fill: axis-col, style: "italic", xlabel))
    }
    if ylabel != none {
      place(dx: ax-x * 1pt + 6pt, dy: 2pt,
        text(size: 0.75em, fill: axis-col, style: "italic", ylabel))
    }
  })
}

// ============================================================= note pieces

#let checkbox(checked: false, color: auto) = context {
  let col = if color == auto { ink.get() } else { color }
  box(baseline: 2pt, {
    sketch-rect(9pt, 9pt, color: col, thickness: 0.9pt, amp: 0.35pt, seed: 7)
    if checked {
      place(dx: 1pt, dy: 1pt, curve(
        stroke: _stroke(accent.get(), 1.4pt),
        curve.move((0.5pt, 4.5pt)),
        curve.line((3.2pt, 8pt)),
        curve.line((8.5pt, -1pt)),
      ))
    }
  })
}

#let todo(body) = { checkbox(); h(6pt); body }
#let done(body) = { checkbox(checked: true); h(6pt); text(fill: gray, body) }

#let note-box(body, title: none, color: auto, fill: none, seed: 3, tilt: 0deg) = context {
  let col = if color == auto { ink.get() } else { color }
  sketch-box(color: col, fill: fill, seed: seed, tilt: tilt, {
    if title != none {
      block(below: 0.6em, text(weight: "bold", fill: col, size: 1.05em, title))
    }
    body
  })
}

#let definition(body, term: none) = note-box(
  title: if term != none [Definition — #term] else [Definition],
  fill: rgb("#f4f0ff"), seed: 4, body,
)
#let theorem(body, name: none) = note-box(
  title: if name != none [Theorem (#name)] else [Theorem],
  fill: rgb("#eef4fb"), seed: 14, body,
)
#let proof(body) = context {
  text(style: "italic", fill: ink.get().lighten(15%))[Proof. ]
  body
  h(1fr)
  circled(text(size: 0.8em)[∎], seed: 19)
}
#let example-box(body) = note-box(title: [Example], fill: rgb("#f0f8f2"), seed: 8, body)
#let warning(body) = context note-box(
  title: [Careful], color: accent.get(), fill: rgb("#fdf1f1"), seed: 11, body,
)

// ------------------------------------------------------------- page furniture

#let _dots(color, step: 5mm) = tiling(
  size: (step, step),
  place(dx: step / 2, dy: step / 2, circle(radius: 0.5pt, fill: color, stroke: none)),
)

#let _ruled(color, step: 7mm) = tiling(
  size: (10mm, step),
  place(dy: step - 0.3pt, line(length: 10mm, stroke: 0.4pt + color)),
)

#let _binding(x, color) = context layout(size => {
  let n = calc.floor((size.height - 2cm) / 1.6cm)
  for i in range(n + 1) {
    let y = 1.6cm + i * 1.6cm
    place(dx: x - 1pt, dy: y - 5pt,
      rotate(-14deg, ellipse(width: 22pt, height: 11pt,
        stroke: (paint: color, thickness: 2.4pt), fill: none)))
    place(dx: x + 4pt, dy: y - 2.6pt,
      circle(radius: 2.6pt, fill: white, stroke: (paint: color.darken(20%), thickness: 0.6pt)))
  }
})

// ------------------------------------------------------------------ template

#let jotter-notes(
  title: none,
  subtitle: none,
  author: none,
  date: none,
  paper: "a4",
  binding: false,
  dots: true,
  ruled: false,
  pen: rgb("#28356b"),
  marker: rgb("#d94f4f"),
  paper-color: rgb("#fdfdfa"),
  fonts: (
    body: ("Kalam", "Comic Neue", "DejaVu Sans"),
    mono: ("Fantasque Sans Mono", "DejaVu Sans Mono"),
    math: ("Pennstander Math", "New Computer Modern Math"),
  ),
  size: 12pt,
  leading: 0.95em,
  tracking: 0pt,
  spacing: 0pt,
  margin-space: false,
  body,
) = {
  ink.update(pen)
  accent.update(marker)

  let left-margin = if binding { 3.2cm } else { 2.2cm }
  let right-margin = if margin-space { 4.6cm } else { 2.2cm }

  set page(
    paper: paper,
    margin: (left: left-margin, right: right-margin, top: 2.4cm, bottom: 2.4cm),
    fill: paper-color,
    background: {
      if dots { rect(width: 100%, height: 100%, fill: _dots(rgb("#c9c9c4"))) }
      if ruled { rect(width: 100%, height: 100%, fill: _ruled(rgb("#cfd8e6"))) }
      if binding { _binding(1.1cm, rgb("#8f9299")) }
    },
    footer: context {
      set text(size: 0.8em, fill: pen.lighten(25%))
      align(right, box(width: 22pt, height: 18pt, {
        place(top + left, sketch-rect(22pt, 18pt, color: pen.lighten(40%),
          thickness: 0.8pt, amp: 0.4pt, seed: 13))
        place(center + horizon, counter(page).display())
      }))
    },
  )

  set text(font: fonts.body, size: size, fill: pen, tracking: tracking,
    spacing: 100% + spacing)
  show raw: set text(font: fonts.mono)
  show math.equation: set text(font: fonts.math)
  set par(leading: leading, justify: false, spacing: 1.4em)
  set list(marker: text(fill: marker, [•]))
  set enum(numbering: n => text(fill: marker, [#n.]))

  // headings: tiny alternating tilt + varied underline styles
  show heading: it => {
    let lvl = it.level
    let s = (1.9em, 1.4em, 1.15em, 1.05em).at(calc.min(lvl, 4) - 1)
    let col = if lvl == 1 { pen } else { pen.lighten(10%) }
    context {
      let sd = here().position().y.pt()
      let tilt = (_rand(sd) * 1.2 - 0.6) * 1deg
      block(above: 1.6em, below: 0.9em, rotate(tilt, origin: left, {
        set text(size: s, fill: col, weight: if lvl <= 2 { "bold" } else { "medium" })
        it.body
        if lvl <= 2 {
          let w = measure(text(weight: "bold", it.body)).width
          place(dy: 0.22em, sketch-line(
            w,
            color: if lvl == 1 { marker } else { marker.lighten(25%) },
            thickness: if lvl == 1 { 1.6pt } else { 1.1pt },
            style: if lvl == 2 { "wave" } else { "line" },
            seed: sd,
          ))
        }
      }))
    }
  }

  show strong: it => text(fill: marker, weight: "bold", it.body)
  show emph: it => text(style: "italic", it.body)
  show link: it => text(fill: rgb("#2a7fbf"), underline(it))

  show raw.where(block: true): it => sketch-box(
    fill: rgb("#f5f3ec"), color: pen.lighten(35%), seed: 9,
    { set text(size: 0.9em); it },
  )

  show quote.where(block: true): it => block(
    inset: (left: 14pt),
    stroke: (left: (paint: marker.lighten(30%), thickness: 2.5pt)),
    { set text(style: "italic"); it.body },
  )

  if title != none {
    block(above: 0pt, below: 1.8em, {
      set text(size: 2.2em, weight: "bold")
      title
      context {
        let w = measure(text(weight: "bold", title)).width
        place(dy: 0.2em, sketch-line(w * 1.03, color: marker, thickness: 2pt,
          amp: 0.9pt, seed: 1))
      }
      if subtitle != none {
        linebreak()
        text(size: 0.5em, fill: pen.lighten(25%), style: "italic", subtitle)
      }
      if author != none or date != none {
        linebreak()
        text(size: 0.42em, fill: pen.lighten(30%), {
          if author != none { author }
          if author != none and date != none [ #h(6pt)·#h(6pt) ]
          if date != none { date }
        })
      }
    })
  }

  body
}
