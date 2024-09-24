namespace rsl.Tokenizer;

public class TextStream
{
    private readonly Queue<string> _lines;
    public readonly string SourcePath;
    private string _currentLine = "";
    public uint ColumnNumber;
    public uint LineNumber;


    public TextStream(string path) : this(File.ReadAllLines(path), path)
    {
    }

    public TextStream(IEnumerable<string> data, string file)
    {
        SourcePath = file;
        _lines = data.ToQueue();
        ColumnNumber = 0;
        LineNumber = 0;
        if (_lines.Count <= 0) return;
        _currentLine = _lines.Dequeue();
        LineNumber = 1;
    }


    public char? Peak()
    {
        if (IsEmpty()) return null;
        while (_currentLine == "")
        {
            if (IsEmpty())
            {
                return null;
            }

            _currentLine = _lines.Dequeue();
            LineNumber++;
            ColumnNumber = 0;
        }
        
        return _currentLine.FirstOrDefault();
    }

    public string GetRemainingOnLine()
    {
        var result = _currentLine;
        _currentLine = "";
        return result;
    }

    public bool Get(out char result)
    {
        while (_currentLine == "")
        {
            if (IsEmpty())
            {
                result = '\0';
                return false;
            }

            _currentLine = _lines.Dequeue();
            LineNumber++;
            ColumnNumber = 0;
        }


        result = _currentLine.First();
        _currentLine = _currentLine[1..];
        ColumnNumber++;

        return true;
    }

    public bool IsEmpty()
    {
        return _lines.Count == 0 && _currentLine.Length == 0;
    }
}