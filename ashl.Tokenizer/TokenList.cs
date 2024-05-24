using System.Diagnostics.CodeAnalysis;

namespace ashl.Tokenizer;

public sealed class TokenList<T> where T : TokenBase
{
    
    private readonly LinkedList<T> _list = new();
    
    public Exception CreateException(string message, T token)
    {
        return new Exception();
    }
    public void ThrowExpectedInput()
    {
        throw new Exception("Expected Input");
    }
    public T RemoveFront()
    {
        if (Empty())
        {
            ThrowExpectedInput();
        }

        var a = Front();
        _list.RemoveFirst();
        /*if (NotEmpty())
        {

        }*/
        return a;
    }

    public T RemoveBack()
    {
        if (Empty())
        {
            ThrowExpectedInput();
        }

        var a = Back();
        _list.RemoveLast();
        return a;
    }
    
    // public TokenList<T> ExpectFront(TokenType type)
    // {
    //     if (Empty())
    //     {
    //         ThrowExpectedInput();
    //     }
    //
    //     var a = Front();
    //     
    //     if(a.Type != type) throw CreateException("Unexpected input",a);
    //     return this;
    // }
    
    public TokenList<T> ExpectFront(params TokenType[] type)
    {
        if (Empty())
        {
            ThrowExpectedInput();
        }

        var a = Front();
        
        if(!type.Contains(a.Type)) throw CreateException("Unexpected input",a);
        return this;
    }

    // public TokenList<T> ExpectBack(TokenType type)
    // {
    //     if (Empty())
    //     {
    //         ThrowExpectedInput();
    //     }
    //
    //     var a = Back();
    //     
    //     if(a.Type != type) throw CreateException("Unexpected input",a);
    //     return this;
    // }
    public TokenList<T> ExpectBack(params TokenType[] type)
    {
        if (Empty())
        {
            ThrowExpectedInput();
        }

        var a = Back();
        
        if(!type.Contains(a.Type)) throw CreateException("Unexpected input",a);
        return this;
    }

    public T Front()
    {
        if (Empty())
        {
            ThrowExpectedInput();
        }

        return _list.First();
    }

    public T Back()
    {
        if (Empty())
        {
            ThrowExpectedInput();
        }

        return _list.Last();
    }

    public int Size() => _list.Count;

    public TokenList<T> InsertFront(TokenList<T> other)
    {
        while (other.NotEmpty())
        {
            _list.AddFirst(other.Back());
            other.RemoveBack();
        }
        return this;
    }
    
    public TokenList<T> InsertFront(T token)
    {
        _list.AddFirst(token);
        return this;
    }
    
    public TokenList<T> InsertBack(TokenList<T> other)
    {
        while (other.NotEmpty())
        {
            _list.AddLast(other.Front());
            other.RemoveFront();
        }
        return this;
    }
    
    public TokenList<T> InsertBack(T other)
    {
        _list.AddLast(other);
        return this;
    }
    
    public TokenList<T> Clear()
    {
        _list.Clear();
        return this;
    }

    public bool Empty() => Size() == 0;
    public bool NotEmpty() => Size() > 0;

    public TokenList<T> CreateEmpty()
    {
        return new TokenList<T>();
    }
}