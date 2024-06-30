Adding a SQLite Property
========================

To add another property to the preferences database, just edit the SQLite file from the command line. 
Be sure to install the SQLite CLI first. 

.. code-block:: console

    sqlite3 C:\Users\Me\.compucell3d_py3\_settings.sqlite <--Adjust the path if on Mac/Linux

    INSERT INTO settings (name,type,value) VALUES('FrameRate','int',4);
    SELECT * FROM settings where name='FrameRate'; <--Confirms that the command has succeeded

    INSERT INTO settings (name,type,value) VALUES('Quality','int',7);
    SELECT * FROM settings where name='Quality';

    INSERT INTO settings (name,type,value) VALUES('AutomaticMovie','bool',True);
    SELECT * FROM settings where name='AutomaticMovie';

    INSERT INTO settings (name,type,value) VALUES('FfmpegLocation','str','');

    INSERT INTO settings (name,type,value) VALUES('WriteMovieMCS','bool',True);

    
When you are ready to commit your changes, copy the local version of your SQLite file to the version on your repository, for example, ``C:\Users\Me\Documents\cc3d\CompuCell3D\cc3d\core\Configuration_settings\_settings.sqlite``. 
You may also want to edit the same file for OSX: ``CompuCell3D\cc3d\core\Configuration_settings\osx\_settings.sqlite``

That's it! When the next version is deployed, Conda will handle placing the latest version of the SQLite file onto users' machines. 
