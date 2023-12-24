/**
 * @module ol/source/DataTile
 */
import DataTile from '../DataTile.js';
import EventType from '../events/EventType.js';
import SourceState from './State.js';
import TileCache from '../TileCache.js';
import TileEventType from './TileEventType.js';
import TileQueue from '../TileQueue.js';
import TileSource, {TileSourceEvent} from './Tile.js';
import TileState from '../TileState.js';
import XYZ from './XYZ.js';
import {ENABLE_RASTER_REPROJECTION} from '../reproj/common.js';
import {VOID, toPromise} from '../functions.js';
import {assign} from '../obj.js';
import {
  createXYZ,
  extentFromProjection,
  getForProjection as getTileGridForProjection,
} from '../tilegrid.js';
import {equivalent, get as getProjection} from '../proj.js';
import {getKeyZXY} from '../tilecoord.js';
import {getUid} from '../util.js';
import {listen, unlistenByKey} from '../events.js';
import {toSize} from '../size.js';

/**
 * Data tile loading function.  The function is called with z, x, and y tile coordinates and
 * returns {@link import("../DataTile.js").Data data} for a tile or a promise for the same.
 * @typedef {function(number, number, number) : (import("../DataTile.js").Data|Promise<import("../DataTile.js").Data>)} Loader
 */

/**
 * @typedef {Object} Options
 * @property {Loader} [loader] Data loader.  Called with z, x, and y tile coordinates.
 * Returns {@link import("../DataTile.js").Data data} for a tile or a promise for the same.
 * @property {import("./Source.js").AttributionLike} [attributions] Attributions.
 * @property {boolean} [attributionsCollapsible=true] Attributions are collapsible.
 * @property {number} [maxZoom=42] Optional max zoom level. Not used if `tileGrid` is provided.
 * @property {number} [minZoom=0] Optional min zoom level. Not used if `tileGrid` is provided.
 * @property {number|import("../size.js").Size} [tileSize=[256, 256]] The pixel width and height of the source tiles.
 * This may be different than the rendered pixel size if a `tileGrid` is provided.
 * @property {number} [gutter=0] The size in pixels of the gutter around data tiles to ignore.
 * This allows artifacts of rendering at tile edges to be ignored.
 * Supported data should be wider and taller than the tile size by a value of `2 x gutter`.
 * @property {number} [maxResolution] Optional tile grid resolution at level zero. Not used if `tileGrid` is provided.
 * @property {import("../proj.js").ProjectionLike} [projection='EPSG:3857'] Tile projection.
 * @property {import("../tilegrid/TileGrid.js").default} [tileGrid] Tile grid.
 * @property {boolean} [opaque=false] Whether the layer is opaque.
 * @property {import("./State.js").default} [state] The source state.
 * @property {number} [tilePixelRatio] Deprecated.  To have tiles scaled, pass a `tileSize` representing
 * the source tile size and a `tileGrid` with the desired rendered tile size.
 * @property {boolean} [wrapX=false] Render tiles beyond the antimeridian.
 * @property {number} [transition] Transition time when fading in new tiles (in miliseconds).
 * @property {number} [bandCount=4] Number of bands represented in the data.
 * @property {boolean} [interpolate=false] Use interpolated values when resampling.  By default,
 * the nearest neighbor is used when resampling.
 */

/**
 * @typedef {Object} GetTileDataOptions
 * @property {number} [pixelRatio=1] Pixel ratio.
 */

const ENABLE_DATA_REPROJECTION = true;

/**
 * @classdesc
 * A source for typed array data tiles.
 *
 * @fires import("./Tile.js").TileSourceEvent
 * @api
 */
class DataTileSource extends TileSource {
  /**
   * @param {Options} options Image tile options.
   */
  constructor(options) {
    const projection =
      options.projection === undefined ? 'EPSG:3857' : options.projection;

    let tileGrid = options.tileGrid;
    if (tileGrid === undefined && projection) {
      tileGrid = createXYZ({
        extent: extentFromProjection(projection),
        maxResolution: options.maxResolution,
        maxZoom: options.maxZoom,
        minZoom: options.minZoom,
        tileSize: options.tileSize,
      });
    }

    super({
      cacheSize: 0.1, // don't cache on the source
      attributions: options.attributions,
      attributionsCollapsible: options.attributionsCollapsible,
      projection: projection,
      tileGrid: tileGrid,
      opaque: options.opaque,
      state: options.state,
      tilePixelRatio: options.tilePixelRatio,
      wrapX: options.wrapX,
      transition: options.transition,
      interpolate: options.interpolate,
    });

    /**
     * @private
     * @type {number}
     */
    this.gutter_ = options.gutter !== undefined ? options.gutter : 0;

    /**
     * @private
     * @type {import('../size.js').Size|null}
     */
    this.tileSize_ = options.tileSize ? toSize(options.tileSize) : null;
    if (!this.tileSize_ && options.tilePixelRatio && tileGrid) {
      const renderTileSize = toSize(tileGrid.getTileSize(0));
      this.tileSize_ = [
        renderTileSize[0] * options.tilePixelRatio,
        renderTileSize[1] * options.tilePixelRatio,
      ];
    }

    /**
     * @private
     * @type {Array<import('../size.js').Size>|null}
     */
    this.tileSizes_ = null;

    /**
     * @private
     * @type {!Object<string, boolean>}
     */
    this.tileLoadingKeys_ = {};

    /**
     * @private
     */
    this.loader_ = options.loader;

    this.handleTileChange_ = this.handleTileChange_.bind(this);

    /**
     * @type {number}
     */
    this.bandCount = options.bandCount === undefined ? 4 : options.bandCount; // assume RGBA if undefined

    /**
     * @protected
     * @type {!Object<string, import("../tilegrid/TileGrid.js").default>}
     */
    this.tileGridForProjection = {};

    /**
     * @protected
     * @type {!Object<string, import("../TileCache.js").default>}
     */
    this.tileCacheForProjection = {};

    /**
     * Queue for getTileData()
     * @private
     * @type {import("../TileQueue.js").default}
     */
    this.tileQueue_;

    /**
     * @private
     * @type {typeof Float32Array|typeof Uint8Array}
     */
    this.dataType_;

    /**
     * @private
     * @type {number}
     */
    this.bytesPerPixel_;

    /**
     * @private
     * @type {!Array<import("./TileImage.js").default>}
     */
    this.reprojImageSources_;

    /**
     * @private
     * @type {!Object<string, DataTileSource>}
     */
    this.reprojDataSources_;
  }

  /**
   * Set the source tile sizes.  The length of the array is expected to match the number of
   * levels in the tile grid.
   * @protected
   * @param {Array<import('../size.js').Size>} tileSizes An array of tile sizes.
   */
  setTileSizes(tileSizes) {
    this.tileSizes_ = tileSizes;
  }

  /**
   * Get the source tile size at the given zoom level.  This may be different than the rendered tile
   * size.
   * @protected
   * @param {number} z Tile zoom level.
   * @return {import('../size.js').Size} The source tile size.
   */
  getTileSize(z) {
    if (this.tileSizes_) {
      return this.tileSizes_[z];
    }
    if (this.tileSize_) {
      return this.tileSize_;
    }
    const tileGrid = this.getTileGrid();
    return tileGrid ? toSize(tileGrid.getTileSize(z)) : [256, 256];
  }

  /**
   * @param {import("../proj/Projection.js").default} projection Projection.
   * @return {number} Gutter.
   */
  getGutterForProjection(projection) {
    const thisProj = this.getProjection();
    if (!thisProj || equivalent(thisProj, projection)) {
      return this.gutter_;
    } else {
      return 0;
    }
  }

  /**
   * @param {Loader} loader The data loader.
   * @protected
   */
  setLoader(loader) {
    this.loader_ = loader;
  }

  /**
   * @param {number} z Tile coordinate z.
   * @param {number} x Tile coordinate x.
   * @param {number} y Tile coordinate y.
   * @param {number} pixelRatio Pixel ratio.
   * @param {import("../proj/Projection.js").default} projection Projection.
   * @return {!DataTile} Tile.
   */
  getTile(z, x, y, pixelRatio, projection) {
    const thisProj = this.getProjection();
    if (
      ENABLE_DATA_REPROJECTION &&
      ENABLE_RASTER_REPROJECTION &&
      thisProj &&
      projection &&
      !equivalent(thisProj, projection)
    ) {
      const tileCoordKey = getKeyZXY(z, x, y);
      const tileCache = this.getTileCacheForProjection(projection);
      if (tileCache.containsKey(tileCoordKey)) {
        return tileCache.get(tileCoordKey);
      }

      const self = this;
      const loader = function () {
        return self
          .getReprojTileData_([z, x, y], pixelRatio, projection)
          .then(function (data) {
            if (!data) {
              return Promise.reject();
            }
            return data;
          });
      };

      const tileGrid = self.getTileGrid();
      const tilePixelRatio = Math.max.apply(
        null,
        tileGrid.getResolutions().map(function (r, z) {
          const tileSize = toSize(tileGrid.getTileSize(z));
          const textureSize = self.getTileSize(z);
          return Math.max(
            textureSize[0] / tileSize[0],
            textureSize[1] / tileSize[1]
          );
        })
      );
      const reprojTileGrid = self.getTileGridForProjection(projection);
      const tileSize = toSize(reprojTileGrid.getTileSize(z));
      const size = [
        Math.round(tileSize[0] * tilePixelRatio),
        Math.round(tileSize[1] * tilePixelRatio),
      ];

      const tile = new DataTile(
        assign(
          {tileCoord: [z, x, y], loader: loader, size: size},
          this.tileOptions
        )
      );
      tile.key = this.getKey();
      tile.addEventListener(EventType.CHANGE, this.handleTileChange_);

      tileCache.set(tileCoordKey, tile);
      return tile;
    }

    const size = this.getTileSize(z);
    const tileCoordKey = getKeyZXY(z, x, y);
    if (this.tileCache.containsKey(tileCoordKey)) {
      return this.tileCache.get(tileCoordKey);
    }

    const sourceLoader = this.loader_;

    function loader() {
      return toPromise(function () {
        return sourceLoader(z, x, y);
      });
    }

    const options = assign(
      {
        tileCoord: [z, x, y],
        loader: loader,
        size: size,
      },
      this.tileOptions
    );

    const tile = new DataTile(options);
    tile.key = this.getKey();
    tile.addEventListener(EventType.CHANGE, this.handleTileChange_);

    this.tileCache.set(tileCoordKey, tile);
    return tile;
  }

  /**
   * Handle tile change events.
   * @param {import("../events/Event.js").default} event Event.
   */
  handleTileChange_(event) {
    const tile = /** @type {import("../Tile.js").default} */ (event.target);
    const uid = getUid(tile);
    const tileState = tile.getState();
    let type;
    if (tileState == TileState.LOADING) {
      this.tileLoadingKeys_[uid] = true;
      type = TileEventType.TILELOADSTART;
    } else if (uid in this.tileLoadingKeys_) {
      delete this.tileLoadingKeys_[uid];
      type =
        tileState == TileState.ERROR
          ? TileEventType.TILELOADERROR
          : tileState == TileState.LOADED
          ? TileEventType.TILELOADEND
          : undefined;
    }
    if (type) {
      this.dispatchEvent(new TileSourceEvent(type, tile));
    }
  }

  /**
   * Utility method to obtain the data for a single tile, loading if necessary.
   *
   * @param {import("../tilecoord.js").TileCoord} tileCoord Tile Coordinate.
   * @param {GetTileDataOptions} [opt_options] Options.
   * @return {Promise<import("../DataTile.js").Data|undefined>} Data.
   */
  getTileData(tileCoord, opt_options) {
    const options = opt_options || {};
    const pixelRatio = options.pixelRatio || 1;
    const self = this;

    if (!self.tileQueue_) {
      self.tileQueue_ = new TileQueue(function () {
        return 1;
      }, VOID);
    }

    return new Promise(function (resolve, reject) {
      let sourceState = self.getState();

      const readTile = function () {
        if (sourceState === SourceState.READY) {
          const projection = self.getProjection();
          const tile = self.getTile(
            tileCoord[0],
            tileCoord[1],
            tileCoord[2],
            pixelRatio,
            projection
          );
          if (tile) {
            const maxQueue = 8;
            const tileState = tile.getState();
            if (tileState === TileState.LOADED) {
              resolve(tile.getData());
            } else if (tileState === TileState.EMPTY) {
              resolve(undefined);
            } else {
              const key = listen(tile, EventType.CHANGE, function () {
                const tileState = tile.getState();
                if (tileState !== TileState.LOADING) {
                  unlistenByKey(key);
                  self.tileQueue_.loadMoreTiles(maxQueue, maxQueue);
                  resolve(
                    tileState === TileState.LOADED ? tile.getData() : undefined
                  );
                }
              });
              if (tileState === TileState.ERROR) {
                tile.state = TileState.IDLE;
              }
              const tileQueueKey = tile.getKey();
              if (!self.tileQueue_.isKeyQueued(tileQueueKey)) {
                self.tileQueue_.enqueue([tile]);
                self.tileQueue_.loadMoreTiles(maxQueue, maxQueue);
              }
            }
            return;
          }
        }
        resolve(undefined);
      };

      if (sourceState !== SourceState.LOADING) {
        readTile();
      } else {
        const key = listen(self, EventType.CHANGE, function () {
          sourceState = self.getState();
          if (sourceState !== SourceState.LOADING) {
            unlistenByKey(key);
            readTile();
          }
        });
      }
    });
  }

  /**
   * @param {import("../proj/Projection.js").default} projection Projection.
   * @return {!import("../tilegrid/TileGrid.js").default} Tile grid.
   */
  getTileGridForProjection(projection) {
    if (!ENABLE_DATA_REPROJECTION || !ENABLE_RASTER_REPROJECTION) {
      return super.getTileGridForProjection(projection);
    }
    const thisProj = this.getProjection();
    if (this.tileGrid && (!thisProj || equivalent(thisProj, projection))) {
      return this.tileGrid;
    } else {
      const projKey = getUid(projection);
      if (!(projKey in this.tileGridForProjection)) {
        this.tileGridForProjection[projKey] =
          getTileGridForProjection(projection);
      }
      return this.tileGridForProjection[projKey];
    }
  }

  /**
   * Sets the tile grid to use when reprojecting the tiles to the given
   * projection instead of the default tile grid for the projection.
   *
   * This can be useful when the default tile grid cannot be created
   * (e.g. projection has no extent defined) or
   * for optimization reasons (custom tile size, resolutions, ...).
   *
   * @param {import("../proj.js").ProjectionLike} projection Projection.
   * @param {import("../tilegrid/TileGrid.js").default} tilegrid Tile grid to use for the projection.
   * @api
   */
  setTileGridForProjection(projection, tilegrid) {
    if (ENABLE_DATA_REPROJECTION && ENABLE_RASTER_REPROJECTION) {
      const proj = getProjection(projection);
      if (proj) {
        const projKey = getUid(proj);
        if (!(projKey in this.tileGridForProjection)) {
          this.tileGridForProjection[projKey] = tilegrid;
        }
      }
    }
  }

  /**
   * @param {import("../proj/Projection.js").default} projection Projection.
   * @return {import("../TileCache.js").default} Tile cache.
   */
  getTileCacheForProjection(projection) {
    if (!ENABLE_DATA_REPROJECTION || !ENABLE_RASTER_REPROJECTION) {
      return super.getTileCacheForProjection(projection);
    }
    const thisProj = this.getProjection();
    if (!thisProj || equivalent(thisProj, projection)) {
      return this.tileCache;
    } else {
      const projKey = getUid(projection);
      if (!(projKey in this.tileCacheForProjection)) {
        this.tileCacheForProjection[projKey] = new TileCache(0.1); // don't cache
      }
      return this.tileCacheForProjection[projKey];
    }
  }

  /**
   * @param {import("../proj/Projection.js").default} projection Projection.
   * @param {!Object<string, boolean>} usedTiles Used tiles.
   */
  expireCache(projection, usedTiles) {
    if (!ENABLE_DATA_REPROJECTION || !ENABLE_RASTER_REPROJECTION) {
      super.expireCache(projection, usedTiles);
      return;
    }
    const usedTileCache = this.getTileCacheForProjection(projection);

    this.tileCache.expireCache(
      this.tileCache == usedTileCache ? usedTiles : {}
    );
    for (const id in this.tileCacheForProjection) {
      const tileCache = this.tileCacheForProjection[id];
      tileCache.expireCache(tileCache == usedTileCache ? usedTiles : {});
    }
  }

  /**
   * @param {import("../tilecoord.js").TileCoord} tileCoord Tile Coordinate.
   * @param {number} pixelRatio Pixel ratio.
   * @param {import("../proj/Projection.js").default} viewProjection View projection.
   * @return {Promise<import("../DataTile.js").Data|undefined>} Data.
   * @private
   */
  getReprojTileData_(tileCoord, pixelRatio, viewProjection) {
    const self = this;

    const getImageTiles = function (tileCoord, index) {
      return self
        .getTileData(tileCoord, {pixelRatio: pixelRatio})
        .then(function (tileData) {
          if (tileData) {
            const size = self.getTileSize(tileCoord[0]);
            const gutter = self.getGutterForProjection(self.getProjection());
            const pixelSize = [size[0] + 2 * gutter, size[1] + 2 * gutter];
            const isFloat = tileData instanceof Float32Array;
            const pixelCount = pixelSize[0] * pixelSize[1];
            const DataType = isFloat ? Float32Array : Uint8Array;
            const tileDataR = new DataType(tileData.buffer);
            const bytesPerElement = DataType.BYTES_PER_ELEMENT;
            const bytesPerPixel =
              (bytesPerElement * tileDataR.length) / pixelCount;
            const bytesPerRow = tileDataR.byteLength / pixelSize[1];
            const bandCount = Math.floor(
              bytesPerRow / bytesPerElement / pixelSize[0]
            );
            const packedLength = pixelCount * bandCount;
            let packedData = tileDataR;
            if (tileDataR.length !== packedLength) {
              packedData = new DataType(packedLength);
              let dataIndex = 0;
              let rowOffset = 0;
              const colCount = pixelSize[0] * bandCount;
              for (let rowIndex = 0; rowIndex < pixelSize[1]; ++rowIndex) {
                for (let colIndex = 0; colIndex < colCount; ++colIndex) {
                  packedData[dataIndex++] = tileDataR[rowOffset + colIndex];
                }
                rowOffset += bytesPerRow / bytesPerElement;
              }
            }
            const buffer = new Uint8Array(packedData.buffer);
            const canvas = document.createElement('canvas');
            canvas.width = pixelSize[0];
            canvas.height = pixelSize[1];
            const context = canvas.getContext('2d');
            const imageData = context.getImageData(
              0,
              0,
              canvas.width,
              canvas.height
            );
            const data = imageData.data;
            let offset = index * 3;
            for (let j = 0, len = data.length; j < len; j += 4) {
              data[j] = buffer[offset];
              data[j + 1] = buffer[offset + 1];
              data[j + 2] = buffer[offset + 2];
              data[j + 3] = 255;
              offset += bytesPerPixel;
            }
            context.putImageData(imageData, 0, 0);

            self.dataType_ = DataType;
            self.bytesPerPixel_ = bytesPerPixel;
            return canvas;
          }
        });
    };

    function createReprojSource(index) {
      const projection = self.getProjection();
      const tileGrid = self.getTileGrid();
      const tilePixelRatio = Math.max.apply(
        null,
        tileGrid.getResolutions().map(function (r, z) {
          const tileSize = toSize(tileGrid.getTileSize(z));
          const textureSize = self.getTileSize(z);
          return Math.max(
            textureSize[0] / tileSize[0],
            textureSize[1] / tileSize[1]
          );
        })
      );
      const reprojSource = new XYZ({
        projection: projection,
        tileGrid: tileGrid,
        tilePixelRatio: tilePixelRatio,
        url: '{z}/{x}/{y}:' + index,
        cacheSize: 0.1, // don't cache
        gutter: self.getGutterForProjection(projection),
        tileLoadFunction: function (tile) {
          getImageTiles(tile.getTileCoord(), index).then(function (canvas) {
            if (canvas) {
              /** @type {import("../ImageTile.js").default} */ (tile).setImage(
                canvas
              );
            } else {
              tile.setState(TileState.ERROR);
            }
          });
        },
        interpolate: false,
      });
      reprojSource.setTileGridForProjection(
        viewProjection,
        self.getTileGridForProjection(viewProjection)
      );
      return reprojSource;
    }

    if (!self.reprojDataSources_) {
      self.tileCache = new TileCache(0); // cache tiles before reprojection
      self.reprojImageSources_ = [];
      self.reprojDataSources_ = {};
    }
    const projKey = getUid(viewProjection);
    if (!(projKey in self.reprojDataSources_)) {
      self.reprojDataSources_[projKey] = new DataTileSource({
        projection: viewProjection,
        interpolate: self.getInterpolate(),
        tileGrid: self.getTileGridForProjection(viewProjection),
        wrapX: self.getWrapX(),
        loader: function (z, x, y) {
          const canvases = [];

          function handleTile(index) {
            if (index === self.reprojImageSources_.length) {
              self.reprojImageSources_.push(createReprojSource(index));
            }
            return self.reprojImageSources_[index]
              .getTileImage([z, x, y], {
                projection: viewProjection,
              })
              .then(function (canvas) {
                if (!canvas) {
                  return Promise.reject();
                }
                canvases.push(canvas);
                const tilesNeeded = Math.ceil(self.bytesPerPixel_ / 3);
                if (index < tilesNeeded - 1) {
                  return handleTile(index + 1);
                }

                let dataR, dataU;

                for (let tile = canvases.length - 1; tile >= 0; tile--) {
                  const canvas = canvases[tile];

                  if (!dataR) {
                    dataU = new Uint8Array(
                      self.bytesPerPixel_ * canvas.width * canvas.height
                    );
                    dataR = new self.dataType_(dataU.buffer);
                  }
                  const context = canvas.getContext('2d');
                  const data = context.getImageData(
                    0,
                    0,
                    canvas.width,
                    canvas.height
                  ).data;
                  let offset = tile * 3;
                  for (let i = 0, len = data.length; i < len; i += 4) {
                    dataU[offset] = data[i];
                    dataU[offset + 1] = data[i + 1];
                    dataU[offset + 2] = data[i + 2];
                    offset += self.bytesPerPixel_;
                  }
                }
                for (
                  let i = 0, len = self.reprojImageSources_.length;
                  i < len;
                  i++
                ) {
                  self.reprojImageSources_[i].expireCache(viewProjection, {});
                }

                if (!dataR) {
                  return Promise.reject();
                }
                return dataR;
              });
          }
          return handleTile(0);
        },
      });
    }

    const source = self.reprojDataSources_[projKey];
    const wrappedTileCoord = source.getTileCoordForTileUrlFunction(
      tileCoord,
      viewProjection
    );
    return source.getTileData(wrappedTileCoord, {pixelRatio: pixelRatio});
  }
}

export default DataTileSource;
