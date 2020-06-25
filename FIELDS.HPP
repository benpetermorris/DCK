/*-------------------------------------------------------------------------*
 * Custom field definitions.
 *-------------------------------------------------------------------------*/

class fldWallTex : public fldRegion
{
public:
    fldWallTex();

    void SetWall( const char* pszWallName );
    void SetWall( int iWallIndex );
    inline int GetWall()
        { return iWallIndex; }
    virtual void Transfer( int );

protected:
    virtual void Draw();
    int iWallIndex;
    int iOldIndex;
    TEXTURE *pTex;
};


class fldFlatTex : public fldRegion
{
public:
    fldFlatTex();

    void SetFlat( const char* pszFlatName );
    void SetFlat( int iFlatIndex );
    inline int GetFlat()
        { return iFlatIndex; }
    virtual void Transfer( int );

private:
    void Draw();
    int iFlatIndex;
    int iOldIndex;
};
