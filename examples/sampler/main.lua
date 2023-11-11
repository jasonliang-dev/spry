function spry.start()
  font = spry.default_font()

  player = spry.image_load "player.png"

  nearest = spry.make_sampler {
    min_filter = "nearest",
    mag_filter = "nearest",
  }

  linear = spry.make_sampler {
    min_filter = "linear",
    mag_filter = "linear",
  }

  wrap = spry.make_sampler {
    wrap_u = "repeat",
    wrap_v = "repeat",
  }

  mirrored = spry.make_sampler {
    wrap_u = "mirroredrepeat",
    wrap_v = "mirroredrepeat",
  }

  clamp = spry.make_sampler {
    wrap_u = "clamp",
    wrap_v = "clamp",
  }

  linear_clamp = spry.make_sampler {
    min_filter = "linear",
    mag_filter = "linear",
    wrap_u = "clamp",
    wrap_v = "clamp",
  }
end

function spry.frame(dt)
  nearest:use()
  font:draw("Hello, World!", 100, 100, 16)
  linear:use()
  font:draw("Hello, World!", 100, 140, 16)

  local rot = 0
  local sx, sy = 3, 3
  local ox, oy = 0, 0
  local u0, v0, u1, v1 = -0.5, -0.5, 1.5, 1.5

  wrap:use()
  player:draw(200, 200, rot, sx, sy, ox, oy, u0, v0, u1, v1)
  mirrored:use()
  player:draw(300, 300, rot, sx, sy, ox, oy, u0, v0, u1, v1)
  clamp:use()
  player:draw(400, 400, rot, sx, sy, ox, oy, u0, v0, u1, v1)

  linear_clamp:use()
  player:draw(0, 0, rot, sx, sy, ox, oy, u0, v0, u1, v1)

  font:draw(("fps: %.2f (%.4f)"):format(1 / dt, dt * 1000))
end