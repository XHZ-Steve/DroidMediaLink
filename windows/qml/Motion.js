.pragma library

function rate(controller, revision, scope) {
    if (!controller) return 1
    const global = Number(controller.setting("animation/speed", 1))
    const local = Number(controller.setting("animation/" + scope, 1))
    return Math.max(.125, global * local)
}

function duration(milliseconds, rate) {
    return Math.round(milliseconds / rate)
}
