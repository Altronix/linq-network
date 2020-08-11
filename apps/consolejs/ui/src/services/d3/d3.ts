import * as d3Array from 'd3-array';
import * as d3Axis from 'd3-axis';
import * as d3Brush from 'd3-brush';
import * as d3Chord from 'd3-chord';
import * as d3Collection from 'd3-collection';
import * as d3Color from 'd3-color';
import * as d3Contour from 'd3-contour';
import * as d3Dispatch from 'd3-dispatch';
import * as d3Drag from 'd3-drag';
import * as d3Dsv from 'd3-dsv';
import * as d3Ease from 'd3-ease';
import * as d3Fetch from 'd3-fetch';
import * as d3Force from 'd3-force';
import * as d3Format from 'd3-format';
import * as d3Geo from 'd3-geo';
import * as d3Hierarchy from 'd3-hierarchy';
import * as d3Interpolate from 'd3-interpolate';
import * as d3Path from 'd3-path';
import * as d3Polygon from 'd3-polygon';
import * as d3Quadtree from 'd3-quadtree';
import * as d3Random from 'd3-random';
import * as d3ScaleChromatic from 'd3-scale-chromatic';
import * as d3Scale from 'd3-scale';
import * as d3Selection from 'd3-selection';
import * as d3Shape from 'd3-shape';
import * as d3TimeFormat from 'd3-time-format';
import * as d3Time from 'd3-time';
import * as d3Timer from 'd3-timer';
import * as d3Transition from 'd3-transition';
import * as d3Voronoi from 'd3-voronoi';
import * as d3Zoom from 'd3-zoom';

type TArray = typeof d3Array;
type TAxis = typeof d3Axis;
type TBrush = typeof d3Brush;
type TChord = typeof d3Chord;
type TCollection = typeof d3Collection;
type TColor = typeof d3Color;
type TContour = typeof d3Contour;
type TDispatch = typeof d3Dispatch;
type TDrag = typeof d3Drag;
type TDsv = typeof d3Dsv;
type TEase = typeof d3Ease;
type TFetch = typeof d3Fetch;
type TForce = typeof d3Force;
type TFormat = typeof d3Format;
type TGeo = typeof d3Geo;
type THierarchy = typeof d3Hierarchy;
type TInterpolate = typeof d3Interpolate;
type TPath = typeof d3Path;
type TPolygon = typeof d3Polygon;
type TQuadtree = typeof d3Quadtree;
type TRandom = typeof d3Random;
type TScaleChromatic = typeof d3ScaleChromatic;
type TScale = typeof d3Scale;
type TSelection = typeof d3Selection;
type TShape = typeof d3Shape;
type TTimeFormat = typeof d3TimeFormat;
type TTime = typeof d3Time;
type TTimer = typeof d3Timer;
type TTransition = typeof d3Transition;
type TVoronoi = typeof d3Voronoi;
type TZoom = typeof d3Zoom;

export type D3 = TArray &
  TAxis &
  TBrush &
  TChord &
  TCollection &
  TColor &
  TContour &
  TDispatch &
  TDrag &
  TDsv &
  TEase &
  TFetch &
  TForce &
  TFormat &
  TGeo &
  THierarchy &
  TInterpolate &
  TPath &
  TPolygon &
  TQuadtree &
  TRandom &
  TScaleChromatic &
  TScale &
  TSelection &
  TShape &
  TTimeFormat &
  TTime &
  TTimer &
  TTransition &
  TVoronoi &
  TZoom;

export const d3: D3 = {
  ...d3Array,
  ...d3Axis,
  ...d3Brush,
  ...d3Chord,
  ...d3Collection,
  ...d3Color,
  ...d3Contour,
  ...d3Dispatch,
  ...d3Drag,
  ...d3Dsv,
  ...d3Ease,
  ...d3Fetch,
  ...d3Force,
  ...d3Format,
  ...d3Geo,
  ...d3Hierarchy,
  ...d3Interpolate,
  ...d3Path,
  ...d3Polygon,
  ...d3Quadtree,
  ...d3Random,
  ...d3ScaleChromatic,
  ...d3Scale,
  ...d3Selection,
  ...d3Shape,
  ...d3TimeFormat,
  ...d3Time,
  ...d3Timer,
  ...d3Transition,
  ...d3Voronoi,
  ...d3Zoom
};
