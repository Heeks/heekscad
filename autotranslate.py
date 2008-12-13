import translate
import re
import codecs

f_in = open('HeeksCAD.po')
#f_out = codecs.open('de/HeeksCAD.po', encoding='latin_1', mode='w')
#f_out = codecs.open('zh_CN/HeeksCAD.po', encoding='utf_8', mode='w')
#f_out = codecs.open('zh_TW/HeeksCAD.po', encoding='utf_8', mode='w')
# turkish not available yet f_out = codecs.open('tr/HeeksCAD.po', encoding='utf_8', mode='w')
f_out = codecs.open('ru/HeeksCAD.po', encoding='utf_8', mode='w')

translate_str = ''

count = 1

while (True):
    line = f_in.readline();
    if (len(line) == 0) : break;

    if(line.find('msgid') != -1):
        e = re.compile('msgid "(.*)"')
        m = e.search(line)
        if m:
            translate_str = m.group(1).lower()
            #translate_str = unicode(translate_str, encoding='latin_1')
            #translate_str = translate.translate('en', 'de', translate_str)
            translate_str = unicode(translate_str, encoding='utf_8')
            #translate_str = translate.translate('en', 'zh-CN', translate_str)
            #translate_str = translate.translate('en', 'zh-TW', translate_str)
            translate_str = translate.translate('en', 'ru', translate_str)
            if translate_str:
                pass
            else:
                translate_str = ''
        else:
            translate_str = ''

    if(line.find('Content-Type: text/plain') != -1):
        f_out.write('"Content-Type: text/plain; charset=utf-8\\n"')
    elif(line.find('msgstr') != -1):
        new_line = 'msgstr "' + translate_str + '"\n'
        #print new_line,
        print count
        count = count + 1
        f_out.write(new_line)
    else:
        f_out.write(line)

f_in.close();
f_out.close();
