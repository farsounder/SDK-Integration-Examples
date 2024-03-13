/* generated using openapi-typescript-codegen -- do no edit */
/* istanbul ignore file */
/* tslint:disable */
/* eslint-disable */
import type { HistoryData } from '../models/HistoryData';
import type { ProcessorSettings } from '../models/ProcessorSettings';
import type { ProcessorSettingsUpdate } from '../models/ProcessorSettingsUpdate';
import type { TargetData } from '../models/TargetData';

import type { CancelablePromise } from '../core/CancelablePromise';
import type { BaseHttpRequest } from '../core/BaseHttpRequest';

export class DefaultService {

    constructor(public readonly httpRequest: BaseHttpRequest) {}

    /**
     * Redirect to the api documentation at /docs
     * @returns any Successful Response
     * @throws ApiError
     */
    public docsGet(): CancelablePromise<any> {
        return this.httpRequest.request({
            method: 'GET',
            url: '/',
        });
    }

    /**
     * Get the latest realtime detection data
     * Get the latest realtime detection data.
 *
 * This endpoint returns the latest detection data from the realtime processor.
 * This includes realtime detections from the latest ping on any inwater targets
 * as well as the latest detections from the latest ping on any bottom targets.
     * @returns TargetData Successful Response
     * @throws ApiError
     */
    public targetDataApiTargetDataGet(): CancelablePromise<TargetData> {
        return this.httpRequest.request({
            method: 'GET',
            url: '/api/target_data',
        });
    }

    /**
     * Get the latest history data
     * Get the latest Local History Mapping data.
 *
 * This endpoint returns the latest Local History Mapping data from the history
 * database. This includes gridded bottom and inwater detections within the
 * specified radius of the given latitude and longitude. If since_timestamp_utc
 * is specified, then only data that has been collected or updated since that
 * time will be returned. If since_timestamp_utc is not specified, then all the
 * data in the radius will be returned.
     * @param latitude Latitude in decimal degrees
     * @param longitude Longitude in decimal degrees
     * @param radiusMeters Radius within which to query for history data, in Meters
     * @param sinceTimestampUtc Return data that has been collected or updated since this time in UTC seconds since the linux epoch. If it is not specified, then all the data in the radius will be returned.
     * @param skip 
     * @param limit 
     * @returns HistoryData Successful Response
     * @throws ApiError
     */
    public historyDataApiHistoryDataGet(
latitude: number,
longitude: number,
radiusMeters: number = 500,
sinceTimestampUtc?: number,
skip?: number,
limit: number = 500,
): CancelablePromise<HistoryData> {
        return this.httpRequest.request({
            method: 'GET',
            url: '/api/history_data',
            query: {
                'latitude': latitude,
                'longitude': longitude,
                'radius_meters': radiusMeters,
                'since_timestamp_utc': sinceTimestampUtc,
                'skip': skip,
                'limit': limit,
            },
            errors: {
                422: `Validation Error`,
            },
        });
    }

    /**
     * Get the current processor settings
     * Get the latest processor settings.
     * @returns ProcessorSettings Successful Response
     * @throws ApiError
     */
    public processorSettingsApiProcessorSettingsGet(): CancelablePromise<ProcessorSettings> {
        return this.httpRequest.request({
            method: 'GET',
            url: '/api/processor_settings',
        });
    }

    /**
     * Change the current processor settings
     * Update the current processor settings.
 *
 * This endpoint is used to change the current processor settings. The settings
 * included in the request will be updated, the rest will be left unchanged.
     * @param requestBody 
     * @returns ProcessorSettings Successful Response
     * @throws ApiError
     */
    public updateProcessorSettingsApiProcessorSettingsPatch(
requestBody: ProcessorSettingsUpdate,
): CancelablePromise<ProcessorSettings> {
        return this.httpRequest.request({
            method: 'PATCH',
            url: '/api/processor_settings',
            body: requestBody,
            mediaType: 'application/json',
            errors: {
                422: `Validation Error`,
            },
        });
    }

}
