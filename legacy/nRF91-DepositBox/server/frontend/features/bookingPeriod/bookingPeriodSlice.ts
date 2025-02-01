import { createAsyncThunk, createSlice } from "@reduxjs/toolkit"
import axiosService from "../../services/axiosService"
import { BookingPeriodType } from "../../typings/bookingPeriodType"
import { BookingPeriodFilterType } from "../../typings/filterType"

type BookingPeriodState = {
    thisPeriod: BookingPeriodType,
    bookingPeriods: BookingPeriodType[],
    status: string,
    error: string | null
}

const initialState: BookingPeriodState = {
    thisPeriod: {
        name: '',
        startDate: '',
        endDate: '',
        bookingOpens: 8,
        bookingCloses: 22,
        boatID: 0
    },
    bookingPeriods: [],
    status: 'idle',
    error: null
}

export const fetchBookingPeriod = createAsyncThunk('bookingPeriod/fetchBookingPeriod', async (filters: BookingPeriodFilterType) => {
    const axe = axiosService.get(`BookingPeriods?Name=${filters.name}&BoatID=${filters.boatID}/`);
    const response = await axe.then(res => res.data);
    return response;
})

export const postBookingPeriod = createAsyncThunk('bookingPeriod/postBookingPeriod', async (data: BookingPeriodType) => {
    const axe = axiosService.post('BookingPeriods/', data);
    const response = await axe.then(res => res.data);
    return response;
})

export const fetchBookingPeriods = createAsyncThunk('bookingPeriod/fetchBookingPeriods', async (boatID: number) => {
    const axe = axiosService.get(`BookingPeriods/${boatID}`);
    const response = await axe.then(res => res.data);
    return response;
})

export const BookingPeriodSlice = createSlice({
    name: 'bookingPeriod',
    initialState,
    reducers: {
    },
    extraReducers(builder) {
        builder
            .addCase(fetchBookingPeriod.pending, state => {
                state.status = 'loading';
            })
            .addCase(fetchBookingPeriod.fulfilled, (state, action) => {
                state.status = 'succeeded';
                if (action.payload) {
                    state.thisPeriod = action.payload[0];
                }
            })
            .addCase(fetchBookingPeriod.rejected, (state, action) => {
                state.status = 'failed';
                if (action.error.message) {
                    state.error = action.error.message;
                }
            })
            .addCase(postBookingPeriod.pending, state => {
                state.status = 'loading';
            })
            .addCase(postBookingPeriod.fulfilled, (state, action) => {
                if (action.payload) {
                    state.bookingPeriods.push(action.payload);
                }
                state.status = 'succeeded';
            })
            .addCase(postBookingPeriod.rejected, (state, action) => {
                if (action.error.message) {
                    state.error = action.error.message;
                }
                state.status = 'failed';
            })
            .addCase(fetchBookingPeriods.pending, state => {
                state.status = 'loading';
            })
            .addCase(fetchBookingPeriods.fulfilled, (state, action) => {
                if (action.payload) {
                    state.bookingPeriods = action.payload.items;
                }
                state.status = 'succeeded';
            })
            .addCase(fetchBookingPeriods.rejected, (state, action) => {
                if (action.error.message) {
                    state.error = action.error.message;
                }
                state.status = 'failed';
            })
    }
})

export default BookingPeriodSlice.reducer;