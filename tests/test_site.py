import re
import struct
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
SITE = ROOT / "site"


class SiteTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.html = (SITE / "index.html").read_text(encoding="utf-8")
        cls.css = (SITE / "styles.css").read_text(encoding="utf-8")

    def test_local_links_and_images_exist(self):
        paths = re.findall(r'(?:href|src)="([^"]+)"', self.html)
        for value in paths:
            if value.startswith("#"):
                continue
            self.assertTrue((SITE / value).is_file(), value)

    def test_every_image_has_dimensions_and_alt_text(self):
        images = re.findall(r"<img\s+[^>]+>", self.html)
        self.assertGreaterEqual(len(images), 6)
        for tag in images:
            self.assertRegex(tag, r'alt="[^"]+"')
            self.assertIn('width="240"', tag)
            self.assertIn('height="160"', tag)

    def test_native_captures_are_240_by_160_pngs(self):
        for path in sorted((SITE / "assets/screens").glob("*.png")):
            data = path.read_bytes()
            self.assertEqual(data[:8], b"\x89PNG\r\n\x1a\n")
            self.assertEqual(struct.unpack(">II", data[16:24]), (240, 160), path.name)

    def test_screens_use_four_x_nearest_neighbor_scaling(self):
        self.assertIn("width: 960px", self.css)
        self.assertIn("image-rendering: pixelated", self.css)
        self.assertIn("max-width: 100%", self.css)

    def test_showcase_uses_real_sonar_pair(self):
        self.assertIn('data-ping="assets/screens/04-sonar-ping.png"', self.html)
        self.assertIn('data-silent="assets/screens/03-silent-water.png"', self.html)


if __name__ == "__main__":
    unittest.main()
