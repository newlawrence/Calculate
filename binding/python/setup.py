import os.path as path
from setuptools import setup

library = 'calculate'


extensions = ['so', 'dylib', 'dll']
library_name = 'lib' + library
for ext in extensions:
    basedir = path.realpath(__file__).replace(path.basename(__file__), '')
    basedir = path.join(basedir, library)
    library_path = path.join(basedir, library_name + '.' + ext)
    if path.lexists(library_path):
        break

setup(
    name=library,
    version='1.0.0',
    author='Alberto Lorenzo',
    author_email='alorenzo.md@gmail.com',
    home_page='https://github.com/newlawrence/Calculate',
    license='MIT',
    description='Little math expression parser',
    install_requires=['cffi>=1.0.0'],
    packages=[library],
    package_data={library: [library_name + '.' + ext]}
)
