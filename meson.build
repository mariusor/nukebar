project('nukebar', 'c',
        default_options : [
            'c_std=gnu11',
            'buildtype=debug',
            'warning_level=3',
        ],
        license : 'MIT')

c_args = ['-Wall', '-Wno-unused-function', '-Wno-unused-parameter', '-fno-omit-frame-pointer']

bin_name = meson.project_name()

if get_option('buildtype') == 'debug'
    add_project_arguments('-Wextra', language : 'c')
    add_project_arguments('-Wpedantic', language : 'c')
    add_project_arguments('-Werror', language : 'c')
    add_project_arguments('-DDEBUG', language : 'c')
endif
add_project_arguments('-lrt', language: 'c')

wayland_client = dependency('wayland-client', required: true)
wayland_protos = dependency('wayland-protocols', version: '>=1.14', required: true)
cc = meson.get_compiler('c')

subdir('protocol')

deps = [
    wayland_client,
    wayland_protos,
    client_protos
]

version_hash = get_option('version')

prefixdir = get_option('prefix')
if not prefixdir.startswith('/')
    error('Prefix is not absolute: "@0@"'.format(prefixdir))
endif
bindir = join_paths(prefixdir, get_option('bindir'))

srcdir = [
    include_directories('src'),
]

sources = ['src/bar.c']

if version_hash != ''
    add_project_arguments('-DVERSION_HASH="' + version_hash + '"', language : 'c')
else
    git = find_program('git', required : false)
    if git.found()
        git_version = vcs_tag(
            input: 'src/version.h.in',
            output: 'version.h',
            replace_string : '@GIT_VERSION@',
            command : ['git', 'describe', '--tags', '--long', '--dirty=-git', '--always'],
            fallback : '(unknown)')
        sources += git_version
    endif
endif


executable('nukebar',
            sources,
            c_args: c_args,
            include_directories: srcdir,
            install : true,
            install_dir : bindir,
            dependencies: deps
)

scdoc = find_program('scdoc', required: false)
if scdoc.found()
    sh = find_program('sh')
    mandir = get_option('mandir')
    man_files = [
        'doc/nukebar.1.scd',
    ]
    foreach filename : man_files
        topic = filename.split('.')[-3].split('/')[-1]
        section = filename.split('.')[-2]
        output = '@0@.@1@'.format(topic, section)

        custom_target(
            output,
            input: filename,
            output: output,
            command: [
                sh, '-c', '@0@ < @INPUT@ > @1@'.format(scdoc.path(), output)
            ],
            install: true,
            install_dir: '@0@/man@1@'.format(mandir, section)
        )
    endforeach
endif
