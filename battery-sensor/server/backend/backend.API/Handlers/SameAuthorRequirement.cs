using System.Linq;
using System.Security.Claims;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Authorization;

public class DocumentAuthorizationHandler :
    AuthorizationHandler<SameAuthorRequirement, string>
{
    // Same author requirement ensuring that the entity interacted with is either owned by the requesting user, or is an admin 
    protected override Task HandleRequirementAsync(AuthorizationHandlerContext context,
                                                   SameAuthorRequirement requirement,
                                                   string resource)
    {

        var userID = context.User.Claims.FirstOrDefault(c => c.Type == ClaimTypes.NameIdentifier);

        if (userID.Value == resource || context.User.IsInRole("Admin"))
        {
            context.Succeed(requirement);
        }

        return Task.CompletedTask;
    }
}

public class SameAuthorRequirement : IAuthorizationRequirement { }