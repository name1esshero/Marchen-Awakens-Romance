// Capture actual editor pixels. Requires Playwright/Chromium; no DOM artwork replacement.
// Start tests/map_editor_browser_server.py, then pass its session JSON here.
const fs = require('fs');
const path = require('path');
const {chromium} = require(process.env.MAR_PLAYWRIGHT_MODULE || 'playwright-core');
(async () => {
    const session = JSON.parse(fs.readFileSync(process.argv[2] || 'build/map-editor-browser.json'));
    const output = path.resolve('build/map-editor-demo');
    fs.mkdirSync(output, {recursive: true});
    fs.rmSync(path.join(output, 'frames.json'), {force: true});
    const browser = await chromium.launch({headless: true});
    const frames = [];
    const errors = [];
    try {
        const page = await browser.newPage({viewport: {width: 1360, height: 900}, deviceScaleFactor: 1});
        page.on('pageerror', error => errors.push(error.message));
        async function capture(duration = 160) {
            const file = `frame-${String(frames.length).padStart(3, '0')}.png`;
            await page.screenshot({path: path.join(output, file)});
            const state = await page.evaluate(() => ({
                status: document.querySelector('#status')?.textContent,
                map: document.querySelector('#maps')?.value,
                scrollX: document.querySelector('#map-scroll')?.scrollLeft,
                scrollY: document.querySelector('#map-scroll')?.scrollTop,
            }));
            frames.push({file, duration, ...state});
        }
        // Delay actual network delivery so the real loading state can be captured.
        // No API response, DOM, or canvas content is fabricated.
        const network = await page.context().newCDPSession(page);
        await network.send('Network.enable');
        await network.send('Network.emulateNetworkConditions', {
            offline: false, latency: 800, downloadThroughput: -1, uploadThroughput: -1,
        });
        await page.goto(session.url, {waitUntil: 'domcontentloaded'});
        await capture(600);
        if (!frames[0].status?.startsWith('Loading map catalog'))
            throw Error('Initial screenshot missed the actual loading state');
        await network.send('Network.emulateNetworkConditions', {
            offline: false, latency: 0, downloadThroughput: -1, uploadThroughput: -1,
        });
        await page.waitForFunction(() => typeof map !== 'undefined' && map && !busy);
        await capture(1000);
        const box = await page.locator('#map-scroll').boundingBox();
        await page.mouse.move(box.x + box.width / 2, box.y + box.height / 2);
        for (let i = 0; i < 12; i++) {
            await page.locator('#map-scroll').evaluate(el => el.scrollBy(0, 30));
            await page.waitForTimeout(100);
            await capture();
        }
        await capture(600);
        for (let i = 0; i < 10; i++) {
            await page.locator('#map-scroll').evaluate(el => el.scrollBy(30, 0));
            await page.waitForTimeout(100);
            await capture();
        }
        await capture(600);
        for (let i = 0; i < 12; i++) {
            await page.locator('#map-scroll').evaluate(el => el.scrollBy(-30, -30));
            await page.waitForTimeout(100);
            await capture();
        }
        await capture(1000);
        if (errors.length) throw Error(errors.join('\n'));
        if (!frames.some(f => f.scrollX > 0) || !frames.some(f => f.scrollY > 0))
            throw Error('Recording did not capture both scroll directions');
        fs.writeFileSync(path.join(output, 'frames.json'), JSON.stringify({
            source: 'Unmodified Chromium page screenshots of the running map editor',
            viewport: {width: 1360, height: 900}, initialNetworkLatencyMs: 800, errors, frames,
        }, null, 2) + '\n');
        console.log(`Captured ${frames.length} screenshots in ${output}`);
    } finally { await browser.close(); }
})().catch(error => {console.error(error); process.exit(1);});
