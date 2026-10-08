type str12 = string[12];
     str64 = string[63];
     str255= string[255];

procedure TsrCall(TsrName: str12;
                  A,BC,DE,HL: integer;
                  var resA,resBC,resDE,resHL: integer);
var result: integer;
begin
   result:=0;
   inline($21/TsrName/            { LD HL, TsrName }
          $23/                    { INC HL }
          $11/$3e/$4d/            { LD DE, 4D3EH }
          $cd/$ca/$ff/            { CALL FFCAH }
          $3e/$00/                { LD A, 00H }
          $8f/                    { ADC A, A }
          $32/result/             { LD (result), A }
          $20/$22/                { JR NZ, +22H }

          $c5/                    { PUSH BC }
          $06/$08/                { LD B, 08H }
          $11/$32/$4d/            { LD DE, 4D32H }
          $cd/$ca/$ff/            { CALL FFCAH }
          $dd/$e1/                { POP IX }
          $e5/                    { PUSH HL }
          $fd/$21/*+20/           { LD IY, *+20 }
          $fd/$e3/                { EX (SP), IY }
          $3a/A/                  { LD A, (A) }
          $ed/$4b/BC/             { LD BC, (BC) }
          $ed/$5b/DE/             { LD DE, (DE) }
          $2a/HL/                 { LD HL, (HL) }
          $fd/$e9/                { JP (IY) }

          $32/A/                  { LD (A), A }
          $ed/$43/BC/             { LD (BC), BC }
          $ed/$53/DE/             { LD (DE), DE }
          $22/HL                  { LD (HL), HL }
          );
   resA:=A;
   resBC:=BC;
   resDE:=DE;
   resHL:=HL;
end;

PROCEDURE LoadGifPicture(DoelX,DoelY,Breedte,Hoogte: INTEGER;
                         var position: REAL;
                         FileName: STR64); EXTERNAL $F9CD; { RST 10H }
PROCEDURE DefGifSize(OffsetX,OffsetY,
                     GifSizeX,GifSizeY,
                     ScreenWidth,ScreenHeight,
                     Condition: INTEGER); EXTERNAL $F9CE; { RST 10H }
PROCEDURE DefGif2(VRAMaddress: INTEGER;
                  InterlacedFormat: BOOLEAN;
                  CodeSize: BYTE); EXTERNAL $F9CF; { RST 10H }

PROCEDURE FWrite(VAR woord : str255); EXTERNAL $F99A; { RST 10H }

procedure CheckEnv;
var i: integer;
begin
   if (not MemmanPresent) then
   begin
      writeln('MemMan not present');
      halt;
   end;
   writeln('MemmanVersion: ',MemmanVersion shr 8,MemmanVersion and 15);
   if (MemmanVersion < $204) then
   begin
      writeln('Current MemMan version: ',
              MemManVersion shr 8,'.',MemManVersion and 15);
      writeln('MemMan 2.4 or higher required');
      halt;
   end;
   if (not GIOSPresent) then
   begin
      writeln('GIOS not present');
      halt;
   end;
   if (GIOSVersion < $215) then
   begin
      writeln('Current GIOS Version: ',
              GIOSVersion shr 8,'.',
              (GIOSVersion shr 4) and 15,
              GIOSVersion and 15);
      writeln('GIOS 2.15 or higher required');
      halt;
   end;

   mem[$f975]:=$11; mem[$f976]:=lo(-$f981); mem[$f977]:=hi(-$f981);     { LD DE, -$f981 }
   mem[$f978]:=$e1;                                                     { POP HL }
   mem[$f979]:=$19;                                                     { ADD HL, DE }
   mem[$f97a]:=$7d;                                                     { LD A, L - toss H away }
   mem[$f97b]:=$2a;                                                     { LD HL, addr(RunGIOS) }
   mem[$f97c]:=lo(addr(RunGIOS));
   mem[$f97d]:=hi(addr(RunGIOS));
   mem[$f97e]:=$e9;                                                     { JP (HL) )

   for i:=$f980 to $f980+100 do mem[i]:=$D7;                            { RST 10H }
   mem[$10]:=$c3; mem[$11]:=$75; mem[$12]:=$f9;                         { JP F975H }

   ClearMem;
end;
