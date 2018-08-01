from conans import ConanFile, CMake


class CalculateConan(ConanFile):
    name = 'Calculate'
    version = '2.1.1'
    license = 'MIT'
    url = 'https://github.com/newlawrence/Calculate.git'
    description = 'Math Expressions Parser Engine'
    exports_sources = 'include/*', 'CMakeLists.txt', 'test/*'
    generators = 'cmake'

    def requirements(self):
        self.requires('catch2/[>2.1,<3.0]@bincrafters/stable')

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build(['--target', 'make_test'])
        cmake.test()

    def package(self):
        self.copy('*.hpp')

    def package_id(self):
        self.info.header_only()
