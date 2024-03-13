/* generated using openapi-typescript-codegen -- do no edit */
/* istanbul ignore file */
/* tslint:disable */
/* eslint-disable */

import type { FOVType } from './FOVType';

/**
 * Data model for updating processor settings. 
 */
export type ProcessorSettingsUpdate = {
    current_squelch_db?: number;
    auto_squelch_mode_enabled?: boolean;
    detect_bottom_enabled?: boolean;
    field_of_view?: FOVType;
};
