/* generated using openapi-typescript-codegen -- do no edit */
/* istanbul ignore file */
/* tslint:disable */
/* eslint-disable */

import type { Detection } from './Detection';
import type { Heading } from './Heading';
import type { InwaterDetectionGroup } from './InwaterDetectionGroup';
import type { Position } from './Position';

/**
 * Data model for target data. 
 */
export type TargetData = {
    timestamp_utc: number;
    serial_number: string;
    heading: Heading;
    position: Position;
    bottom_detections: Array<Detection>;
    inwater_detections: Array<InwaterDetectionGroup>;
    maximum_depth_meters: number;
    maximum_range_index: number;
};
