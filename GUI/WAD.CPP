typedef struct
{
    long    lPos;
    long    lSize;
    WADNAME name;

} WADEntry;

class WAD
{
public:
    WAD();
    ~WAD();

    int Init( const char* wadfn );
    int Open( const char* wadfn, short mode = WAD::waComplete );
    int Load( short mode = WAD::waComplete );
    void Close();

    WADEntry* Find( const char* );
    WADEntry* FindInMap( const char* pszEntry, const char* pszMap = NULL );

    // modes:
    enum
    {
        wlMapOnly,
        wlComplete
    };

    // fail?
    inline bool fail() 
        { return fFailed; }

private:
    WADEntry* 
    bool fFailed;
};
