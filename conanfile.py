from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.microsoft import is_msvc

# check cmake_cxx_extensions


class Exiv2Conan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"

    options = {
        "bmff": [True, False],
        "coverage": [True, False],
        "docs": [True, False],
        "fuzzTests": [True, False],
        "intl": [True, False],
        "samples": [True, False],
        "shared": [True, False],
        "unitTests": [True, False],
        "webready": [True, False],
    }

    default_options = {
        "bmff": True,
        "coverage": False,
        "docs": False,
        "fuzzTests": False,
        "intl": False,
        "samples": True,
        "shared": True,
        "unitTests": True,
        "webready": True,
    }

    exports_sources = "*"

    def validate_build(self):
        check_min_cppstd(self, 17)

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.cache_variables["EXIV2_ENABLE_BMFF"] = self.options.bmff
        tc.cache_variables["BUILD_WITH_COVERAGE"] = self.options.coverage
        tc.cache_variables["EXIV2_BUILD_DOC"] = self.options.docs
        tc.cache_variables["EXIV2_BUILD_FUZZ_TESTS"] = self.options.fuzzTests
        tc.cache_variables["EXIV2_ENABLE_NLS"] = self.options.intl
        tc.cache_variables["EXIV2_BUILD_SAMPLES"] = self.options.samples
        tc.cache_variables["BUILD_SHARED_LIBS"] = self.options.shared
        tc.cache_variables["EXIV2_BUILD_UNIT_TESTS"] = self.options.unitTests
        tc.cache_variables["EXIV2_ENABLE_WEBREADY"] = self.options.webready
        # what happenes if we have webready and no curl?!
        tc.cache_variables["EXIV2_ENABLE_CURL"] = self.options.webready
        tc.generate()
        cmake = CMakeDeps(self)
        cmake.generate()

    def configure(self):
        self.options["libcurl"].shared = True
        self.options["gtest"].shared = False

    def requirements(self):
        self.requires("brotli/1.0.9")
        self.requires("zlib/1.2.13")

        if self.options.webready:
            self.requires("libcurl/7.85.0")

        # we never tested this.. do we need it?
        # if is_msvc(self) and self.options.iconv:
        #     self.requires("libiconv/1.17")

        self.requires("expat/2.4.9")

    def build_requirements(self):
        self.tool_requires("cmake/3.24.2")

        if self.options.intl:
            self.tool_requires("gettext/0.21")

        if self.options.docs:
            self.tool_requires("doxygen/1.9.4")

        if self.options.unitTests:
            self.test_requires("gtest/1.12.1")
            if self.settings.build_type == "Debug":
                self.options["gtest"].debug_postfix = ""



    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
