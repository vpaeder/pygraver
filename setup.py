import sys

try:
    from skbuild import setup
except ImportError:
    print(
        "Please update pip, you need pip 10 or greater,\n"
        " or you need to install the PEP 518 requirements in pyproject.toml yourself",
        file=sys.stderr,
    )
    raise

setup(
    name="PyGraver",
    version="0.1.0",
    description="a toolkit to generate and display toolpaths for a 4-axis engraver with a basic machine interface",
    author="Vincent Paeder",
    license="MIT",
    packages=["pygraver","tests"],
    cmake_install_dir="pygraver",
    extras_require={"tests": ["unittest"]},
    python_requires=">=3.7",
    cmake_args=["-DBUILD_TESTS:BOOL=OFF"]
)
