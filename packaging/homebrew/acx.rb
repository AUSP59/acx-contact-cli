class Acx < Formula
  desc "Audit-ready contact CLI"
  homepage "https://example.org/acx"
  url "https://example.org/downloads/acx.tar.gz"
  version "1.0.0"
  license "Apache-2.0"
  depends_on "cmake" => :build
  depends_on "ninja" => :build
  def install
    system "cmake", "-S", ".", "-B", "build", "-G", "Ninja", "-DCMAKE_BUILD_TYPE=Release"
    system "cmake", "--build", "build", "--target", "acx"
    bin.install "build/acx"
    man1.install "man/acx.1"
    bash_completion.install "completions/acx.bash" => "acx"
  end
  test do
    system "#{bin}/acx", "help"
  end
end
