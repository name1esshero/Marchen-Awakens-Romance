// Exercise the actual inspector against byte- and word-attribute maps.
const assert = require('assert');
const fs = require('fs');
const vm = require('vm');
const source = fs.readFileSync('tools/map_editor/app.js', 'utf8');
const elements = {};
const context = {
    map: {document: {attributes: {word_size: 2, entries: [400, 400, 5400, 65535]}}},
    $: id => elements[id] ||= {value: '0', addEventListener(type, fn) {this[type] = fn;}},
    options: (element, values) => {element.items = values;},
};
vm.createContext(context);
vm.runInContext(source.slice(source.indexOf('function isConnectionAttribute(')), context);
for (const value of [400, 499, 5400, 5499]) assert(context.isConnectionAttribute(value));
for (const value of [0, 399, 500, 5399, 5500, 65535]) assert(!context.isConnectionAttribute(value));
context.refreshAttributeInspector();
assert(elements['attribute-values'].items[1][1].includes('2 tiles'));
elements['attribute-values'].value = '5400';
elements['attribute-values'].change();
assert.equal(elements.attribute.value, '5400');
assert(elements['attribute-meaning'].textContent.includes('procedural connection'));
elements.attribute.value = '65535';
elements.attribute.input();
assert(elements['attribute-meaning'].textContent.includes('not yet decoded'));
context.map.document.attributes = {word_size: 1, entries: [0, 255]};
context.refreshAttributeInspector();
assert.equal(elements.attribute.max, 255);
assert(elements['attribute-meaning'].textContent.includes('0 to 255'));
// Selection and classification never rewrite map values.
assert.deepEqual(context.map.document.attributes.entries, [0, 255]);
context.map.document.attributes = null;
context.refreshAttributeInspector();
assert.equal(elements['attribute-values'].items.length, 1);
console.log('Map attribute inspector checks passed.');
