class Object
{
public:
    Object()
        { iNumber = iType = -1; }
    Object( int _iNumber, int _iType )
        { iNumber = _iNumber; iType = _iType; }

    short iNumber;
    short iType;

    void SetInvalid()
        { iType = -1; }
    bool IsValid()
        { return (iType != -1); }
    bool operator==(Object& obj)
        { return (obj.iNumber == iNumber &&
                  obj.iType == iType); }
    bool operator!=(Object& obj)
        { return (obj.iNumber != iNumber ||
                  obj.iType != iType); }

    operator LINEDEF*()
        { return &linelist[iNumber]; }
    operator THING*()
        { return &thinglist[iNumber]; }
    operator SIDEDEFM*()
        { return &sidelist[iNumber]; }
    operator SECTOR*()
        { return &sectorlist[iNumber]; }
    operator VERTEX*()
        { return &vlist[iNumber]; }
};


