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
    const launch={headless:true};
    if(process.env.MAR_BROWSER_EXECUTABLE)launch.executablePath=process.env.MAR_BROWSER_EXECUTABLE;
    const browser = await chromium.launch(launch);
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
                mode: document.body.dataset.mode,
                script: typeof script !== 'undefined' ? script?.name : null,
                previewFrame: typeof playback !== 'undefined' ? playback.frame : null,
                actorY: typeof playback !== 'undefined' && playback.actors[0]
                    ? actorCoordinate(playback.actors[0], 'y', playback.frame) : null,
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

        // Open the real field whose battle-event chain establishes Dorothy's
        // literal origin, then demonstrate all five persistent editor modes.
        await page.locator('#maps').selectOption('MAP01_3A.KMP');
        await page.waitForFunction(() => map?.name === 'MAP01_3A.KMP' && !busy);
        await page.locator('#fit-map').click();await capture(700);
        for (const mode of ['collision','connections','scripts','map','events']) {
            await page.locator(`#modes button[data-mode="${mode}"]`).click();
            await page.waitForTimeout(120);await capture(450);
        }

        // Show the recovered cross-script origin in the actual Events UI and
        // start its real animation preview before moving to the literal-motion
        // example below.
        const dorothySource=page.locator('#event-sources .event-source',{hasText:'EV_BA03.SPC'});
        await dorothySource.click();
        await page.waitForFunction(() => script?.name === 'EV_BA03.SPC' && !busy);
        if (!(await page.locator('#sprite-candidates').textContent()).includes('inherits (530, 308) from EV_BA02.SPC'))
            throw Error('Dorothy inherited origin is missing from the real event inspector');
        await page.locator('#preview-play').click();
        await page.waitForTimeout(180);await page.locator('#preview-pause').click();
        await capture(800);

        // EV_ICE02 contains the only currently proven sequence combining a
        // literal initial position and literal SprMove target. Keep its actor
        // in view and use the real Play control to record the 32-frame move.
        await page.locator('#maps').selectOption('MAP04_A.KMP');
        await page.waitForFunction(() => map?.name === 'MAP04_A.KMP' && !busy);
        await page.locator('#modes button[data-mode="scripts"]').click();
        await page.locator('#scripts').selectOption('EV_ICE02.SPC');
        await page.waitForFunction(() => script?.name === 'EV_ICE02.SPC' && !busy);
        await page.locator('#modes button[data-mode="events"]').click();
        await page.locator('#zoom').selectOption('1');
        await page.locator('#map-scroll').evaluate(el => {el.scrollLeft=620;el.scrollTop=1660;});
        await capture(700);
        await page.locator('#preview-speed').selectOption('0.5');
        await page.locator('#preview-play').click();
        await page.waitForTimeout(50);await page.locator('#preview-pause').click();
        for (let frame=0;frame<=32;frame+=4) {
            await page.locator('#preview-time').fill(String(frame));
            await page.locator('#preview-time').dispatchEvent('input');
            await capture(110);
        }
        await capture(800);
        if (errors.length) throw Error(errors.join('\n'));
        if (frames.some(frame => /could not|error/i.test(frame.status || '')))
            throw Error('Recording captured an editor error status');
        for (const mode of ['map','collision','events','connections','scripts'])
            if (!frames.some(frame => frame.mode === mode)) throw Error(`Recording missed ${mode} page`);
        if (!frames.some(frame => frame.script === 'EV_BA03.SPC'))
            throw Error('Recording missed Dorothy inherited-origin playback');
        const actorPositions=frames.map(frame=>frame.actorY).filter(Number.isFinite);
        if (new Set(actorPositions.map(Math.round)).size < 2)
            throw Error('Recording did not capture sprite movement');
        fs.writeFileSync(path.join(output, 'frames.json'), JSON.stringify({
            source: 'Unmodified Chromium page screenshots of the running map editor',
            viewport: {width: 1360, height: 900}, initialNetworkLatencyMs: 800,
            demonstrations: ['five editor modes','Dorothy inherited origin','EV_ICE02 literal movement'],
            errors, frames,
        }, null, 2) + '\n');
        console.log(`Captured ${frames.length} screenshots in ${output}`);
    } finally { await browser.close(); }
})().catch(error => {console.error(error); process.exit(1);});
