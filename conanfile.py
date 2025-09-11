from conan import ConanFile

class AcxRecipe(ConanFile):
    name = "acx"
    version = "1.0.0"
    license = "Apache-2.0"
    url = "https://github.com/your-org/acx"
    description = "ACX Enterprise Contact CLI"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    def layout(self):
        from conan.tools.cmake import cmake_layout
        cmake_layout(self)
    def requirements(self):
        pass
    def build(self):
        pass  # use CMake outside or via toolchain
