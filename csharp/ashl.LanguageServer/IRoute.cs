using WatsonWebserver.Core;

namespace ashl.LanguageServer;

public interface IRoute
{
    public static abstract string Route { get; }
    public abstract static Task Run(HttpContextBase ctx);
}