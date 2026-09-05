# HUSH BELOW! showcase

This is a dependency-free static site, deployed as a Cloudflare Worker at
[hush-below.uppercut-labs.workers.dev](https://hush-below.uppercut-labs.workers.dev).
Serve this directory from any static host or run:

```sh
python3 -m http.server 8080 --directory site
```

All gameplay images are native 240x160 mGBA framebuffer captures displayed at 4x with nearest-neighbor scaling. Run `make site` from the repository root to refresh the ROM and captures.

Deploy with:

```sh
npx wrangler deploy --config site/wrangler.jsonc
```
