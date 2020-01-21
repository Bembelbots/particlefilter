# same as fieldSize from frontend
class FieldSize:
    JRL  = 0
    SPL  = 1
    HTWK = 2

# event stuff, keep in sync! (used for logfiles so far)
class Events:
    event_names = []
    event_names.append("EV_INTIAL")#0
    event_names.append("EV_LOST_GROUND")
    event_names.append("EV_BACK_UP")#5
    event_names.append("EV_FALLEN")
    event_names.append("EV_PENALIZED")
    event_names.append("EV_UNPENALIZED")
    event_names.append("EV_STATE_INITIAL")
    event_names.append("EV_STATE_READY")
    event_names.append("EV_STATE_SET")
    event_names.append("EV_STATE_PLAYING")
    event_names.append("EV_STATE_FINISHED")

    def getId(self, name):
        return self.event_names.index(name)
