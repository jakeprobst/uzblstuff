def tabs(uzbl, keylet):
    f = file("/tmp/uzbltab", "wb")
    str = ("tabtitle %s %s \n" % (uzbl.uzbl_socket,keylet)).encode('utf8')
    f.write(str)
    f.close()
 
def turi(uzbl, keylet):
    f = file("/tmp/uzbltab", "wb")
    str = ("taburi %s %s \n" % (uzbl.uzbl_socket,keylet)).encode('utf8')
    f.write(str)
    f.close()
 
def init(uzbl):
    connects = {"TITLE_CHANGED": tabs,
                "LOAD_COMMIT": turi}
 
    uzbl.connect_dict(connects)
