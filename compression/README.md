**English (translation)**

## This directory contains code for RLE compression.  

## Example:  
### Before:  
```hex
00 01 FF 00 00 FF  
00 01 FF 00 00 FF  
00 01 FF 00 00 FF  
00 01 00 FF 00 FF  
```  
### After:  
```hex
00 03 FF 00 00 FF  
00 01 00 FF 00 FF  
```

<br>

**Русский (original)**

## В этой директории находятся код для сжатия RLE.

## Пример:
### Было: 

```hex
00 01 FF 00 00 FF
00 01 FF 00 00 FF
00 01 FF 00 00 FF
00 01 00 FF 00 FF
```

### Стало: 

```hex
00 03 FF 00 00 FF
00 01 00 FF 00 FF
```