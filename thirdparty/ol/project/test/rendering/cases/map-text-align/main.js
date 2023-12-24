import Map from '../../../../src/ol/Map.js';
import TileLayer from '../../../../src/ol/layer/Tile.js';
import View from '../../../../src/ol/View.js';
import XYZ from '../../../../src/ol/source/XYZ.js';
import {fromLonLat} from '../../../../src/ol/proj.js';

new Map({
  layers: [
    new TileLayer({
      source: new XYZ({
        url: '/data/tiles/satellite/{z}/{x}/{y}.jpg',
      }),
    }),
  ],
  target: 'map',
  view: new View({
    rotation: Math.PI / 3,
    center: fromLonLat([8.6, 50.1]),
    zoom: 3,
  }),
});

render();
