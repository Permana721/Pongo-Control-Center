pkgname=pongo-control-center
pkgver=1.0.0
pkgrel=1
pkgdesc="Interactive RGB Control Center for Axioo Pongo 725 laptops"
arch=('x86_64')
url="https://github.com/Permana721/Pongo-Control-Center"
license=('GPL')
depends=('glibc' 'tuxedo-keyboard-dkms')
makedepends=('git')
install=$pkgname.install
source=("git+https://github.com/Permana721/pongo-control-center.git")
sha256sums=('SKIP')

build() {
    cd "$pkgname"
    gcc main.c -O3 -o pongo-control-center
}

package() {
    cd "$pkgname"
    install -Dm4755 pongo-control-center "$pkgdir/usr/bin/pongo-control-center"
}