pkgname=pongo-control-center
pkgver=1.0.0
pkgrel=1
pkgdesc="Interactive RGB Control Center for Axioo Pongo 725 laptops"
arch=('x86_64')
url="https://github.com/Permana721/Pongo-Control-Center"
license=('GPL')
depends=('glibc' 'clevo-drivers-dkms-git')
makedepends=('git')
install=$pkgname.install
source=("main.c")
sha256sums=('SKIP')

build() {
    gcc main.c -O3 -o pongo-control-center
}

package() {
    install -Dm4755 pongo-control-center "$pkgdir/usr/bin/pongo-control-center"
}