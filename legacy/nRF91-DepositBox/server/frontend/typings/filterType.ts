export type BookingFilterType = {
    week: number,
    month: number,
    userID: number,
    filter: string
}

export type BookingPeriodFilterType = {
    name: string,
    boatID: number
}

export type LockBoxFilterType = {
    LockBoxID: number,
    BookingID: number
}