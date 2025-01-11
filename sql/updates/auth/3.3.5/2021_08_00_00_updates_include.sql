-- updates_include

DELETE FROM updates_include WHERE path = '$/sql/Bots/updates/auth';
INSERT INTO updates_include(path, state) VALUES ('$/sql/Bots/updates/auth', 'RELEASED');