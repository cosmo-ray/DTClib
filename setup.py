from setuptools import setup, Extension

extension = Extension(
    "dtclib",
    sources=["dtc_python.c"],
    extra_compile_args=["-O3", "-I./"],
)

setup(
    ext_modules=[extension],
)
