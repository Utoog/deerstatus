project('deerstatus', 'c', version: '2.0.0')

deer_generator = find_program('gendeerpage.sh')

deer_alive = custom_target(
    'deer_alive.c',
    output: 'deer_alive.c',
    input: 'res/deer_alive.txt',
    command: [deer_generator, 'alive'],
)

deer_dead = custom_target(
    'deer_dead.c',
    output: 'deer_dead.c',
    input: 'res/deer_dead.txt',
    command: [deer_generator, 'dead'],
)

conf_data = configuration_data()
conf_data.set('version', meson.project_version())
configure_file(input: 'version.h.in', output: 'version.h', configuration: conf_data)

src = ['main.c', 'server.c', deer_alive, deer_dead, 'mongoose.c']

executable('deerstatus', sources: src)
