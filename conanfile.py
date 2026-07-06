from conan import ConanFile
from conan.tools.cmake import cmake_layout


class TicketMachineConan(ConanFile):
    name = "ticket-machine"
    version = "0.1.0"

    settings = "os", "compiler", "build_type", "arch"

    generators = (
        "CMakeDeps",
        "CMakeToolchain",
    )

    def requirements(self):
        self.requires("asio/1.38.0")
        self.requires("nlohmann_json/3.12.0")

    def build_requirements(self):
        self.test_requires("gtest/1.17.0")

    def layout(self):
        cmake_layout(self)