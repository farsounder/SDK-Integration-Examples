/* generated using openapi-typescript-codegen -- do no edit */
/* istanbul ignore file */
/* tslint:disable */
/* eslint-disable */

/**
 * Data model for gridded detection data (common to bottom and inwater)
 */
export type GriddedInwaterDetection = {
    timestamp_utc: number;
    latitude_degrees: number;
    longitude_degrees: number;
    grid_interval_meters: number;
    target_strength_db: number;
    is_tide_corrected: boolean;
    uploaded_to_cloud: boolean;
    number_of_points: number;
    maximum_depth_meters: number;
    minimum_depth_meters: number;
};
