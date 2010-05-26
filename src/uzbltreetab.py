

def fifotab(uzbl, keylet):
    uzbl.fifopath = keylet

def tabs(uzbl, keylet):
    try:
        f = file(uzbl.fifopath, "wb")
        str = ("tabtitle %s %s\n" % (uzbl.name, keylet)).encode('utf8')
        f.write(str)
        f.close()
    except: pass
    
def turi(uzbl, keylet):
    try:
        f = file(uzbl.fifopath, "wb")
        str = ("taburi %s %s\n" % (uzbl.name, keylet)).encode('utf8')
        f.write(str)
        f.close()
    except: pass

def instancename(uzbl, keylet):
    uzbl.name = keylet

def init(uzbl):
    print dir(uzbl)
    for i in dir(uzbl):
        try:
            print i + ":", uzbl.__dict__[i]
        except:
            print i + ":", "nope"
    connects = {"TITLE_CHANGED": tabs,
                "LOAD_COMMIT": turi,
                "FIFOTAB": fifotab,
                "INSTANCE_NAME": instancename}

    uzbl.connect_dict(connects)
    uzbl.send("event INSTANCE_NAME @NAME")
