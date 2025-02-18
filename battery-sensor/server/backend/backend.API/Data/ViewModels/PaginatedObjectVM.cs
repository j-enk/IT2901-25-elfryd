using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using backend.API.Data.Models;

namespace backend.API.Data.ViewModels
{
    //from https://learn.microsoft.com/en-us/aspnet/core/data/ef-mvc/sort-filter-page?view=aspnetcore-7.0
    public class PaginatedObject<T>
    {
        public int PageIndex { get; set; }
        public int TotalPages { get; set; }
        public List<T> Items { get; set; }
        public bool HasPreviousPage { get; set; }
        public bool HasNextPage { get; set; }

        public PaginatedObject(PaginatedList<T> items)
        {
            this.HasNextPage = items.HasNextPage;
            this.HasPreviousPage = items.HasPreviousPage;
            this.PageIndex = items.PageIndex;
            this.TotalPages = items.TotalPages;
            this.Items = items;
        }



        public static async Task<PaginatedObject<T>> CreateAsync(IQueryable<T> source, int pageIndex, int pageSize)
        {
            var paginatedList = await PaginatedList<T>.CreateAsync(source, pageIndex, pageSize);
            return new PaginatedObject<T>(paginatedList);
        }


        public PaginatedObject<T2> ConvertTo<T2>(List<T2> items, int count, int pageIndex, int pageSize)
        {
            var paginatedList = new PaginatedList<T2>(items, count, pageIndex, pageSize);
            return new PaginatedObject<T2>(paginatedList);

        }
    }
}