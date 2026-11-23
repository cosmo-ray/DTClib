import os
import platform
import sys
import sysconfig

from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext


class BuildExt(build_ext):
    """Custom build_ext that forces mingw32 on Windows.

    DTClib is a C project: it relies on GNU variadic macros and POSIX
    string helpers, so MSVC is not a supported compiler.
    """

    def finalize_options(self):
        if platform.system() == "Windows" and self.compiler is None:
            self.compiler = "mingw32"
        super().finalize_options()

    def get_libraries(self, ext):
        libraries = build_ext.get_libraries(self, ext)
        # distutils/setuptools hardcodes the Python library name as pythonXY
        # for mingw32, but free-threading builds (3.13+) on Windows ship
        # pythonXYt.lib instead of pythonXY.lib.
        # We only "fix" this for 3.14+ because 3.13t is considered
        # experimental and we prefer not to link against it.
        if (
            getattr(self.compiler, "compiler_type", None) == "mingw32"
            and platform.system() == "Windows"
            and (sys.version_info.major, sys.version_info.minor) >= (3, 14)
        ):
            version_nodot = f"{sys.version_info.major}{sys.version_info.minor}"
            expected = f"python{version_nodot}"
            if expected in libraries:
                for prefix in (sys.base_prefix, sysconfig.get_config_var("prefix")):
                    if not prefix:
                        continue
                    libs_dir = os.path.join(prefix, "libs")
                    if os.path.exists(os.path.join(libs_dir, f"{expected}t.lib")):
                        libraries = [
                            f"{expected}t" if lib == expected else lib
                            for lib in libraries
                        ]
                        break
        return libraries


extension = Extension(
    "dtclib",
    sources=["dtc_python.c"],
    depends=["dtclib.h", "dtc-python.h"],
    extra_compile_args=["-O3", "-I./"],
)

setup(
    ext_modules=[extension],
    cmdclass={"build_ext": BuildExt},
)
