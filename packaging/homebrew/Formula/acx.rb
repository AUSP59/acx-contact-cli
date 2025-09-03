class Acx < Formula
  desc "ACX Enterprise Contact CLI — auditable, portable"
  homepage "https://github.com/your-org/acx"
  url "https://github.com/your-org/acx/releases/download/v1.0.0/acx-1.0.0-macos.zip"
  sha256 "REPLACE_WITH_REAL_CHECKSUM"
  license "Apache-2.0"
  def install
    bin.install "acx"
    man1.install "man/acx.1"
  end
  test do
    system "#{bin}/acx", "help"
  end
end
