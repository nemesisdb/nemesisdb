import logging

name = 'ndb'


class Logger:
  def __init__(self):
    self.logger = None

  def enable(self) -> None:
    self.logger = logging.getLogger(name)
    self.logger.setLevel(logging.ERROR)  

  def debug(self, msg: str):
    if self.logger:
      self.logger.debug(msg)

  def error(self, msg: str):
    if self.logger:
      self.logger.error(msg)


logger = Logger()