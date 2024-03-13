/* generated using openapi-typescript-codegen -- do no edit */
/* istanbul ignore file */
/* tslint:disable */
/* eslint-disable */

import type { FOVType } from './FOVType';
import type { SystemType } from './SystemType';

/**
 * Data model for processor settings. 
 */
export type ProcessorSettings = {
    system_type: SystemType;
    timestamp_utc: number;
    minimum_squelch_db: number;
    maximum_squelch_db: number;
    current_squelch_db: number;
    auto_squelch_mode_enabled: boolean;
    detect_bottom_enabled: boolean;
    field_of_view: FOVType;
};
