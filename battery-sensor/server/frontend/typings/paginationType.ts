export type PaginationType<T> = {
    items: T[],
    pageIndex: number,
    totalPages: number,
    hasPreviousPage: boolean,
    hasNextPage: boolean,
}