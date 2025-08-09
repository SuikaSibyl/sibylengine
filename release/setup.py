from setuptools import setup
from setuptools.dist import Distribution

class BinaryDistribution(Distribution):
    def has_ext_modules(self):
        return True  # forces a platform-specific wheel

setup(
    name="sibylengine",
    version='0.0.4',
    description='Custom Vulkan renderer',
    include_package_data=True,
    distclass=BinaryDistribution,
)
