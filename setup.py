from setuptools import setup, Extension

acdetector = Extension('acdetector', sources=["detector.cpp"], extra_compile_args=['-std=c++11'])
setup(ext_modules=[acdetector])