from conans import ConanFile, CMake


class CalculateConan(ConanFile):
    name = 'Calculate'
    version = '2.1.1'
    license = 'MIT'
    url = 'https://github.com/newlawrence/Calculate.git'
    description = 'Math Expressions Parser Engine'
    generators = 'cmake'
    exports_sources = [
        'copying',
        'CMakeLists.txt',
        'cmake/CalculateConfig.cmake.in',
        'include/*',
        'test/*'
    ]

    def build_requirements(self):
        self.build_requires('Catch2/2.9.1@catchorg/stable')

    def build(self):
        cmake = CMake(self)
        cmake.definitions['CALCULATE_BUILD_TESTS'] = 'ON'
        cmake.configure()
        cmake.build(['--target', 'make_test'])
        cmake.test()

    def package(self):
        self.copy('*.hpp')
        self.copy('copying', dst='.')

    def package_id(self):
        self.info.header_only()
